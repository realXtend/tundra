// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TeleportWidget.h"
#include "Inworld/ControlPanelManager.h"

#include <QPropertyAnimation>
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    TeleportWidget::TeleportWidget(QGraphicsScene *scene, ControlPanelManager *control_panel_manager) : 
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        panel_(control_panel_manager)
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);

        setZValue(100);
        visibility_animation_ = new QPropertyAnimation(this, "opacity", this);
        visibility_animation_->setDuration(500);
        visibility_animation_->setEasingCurve(QEasingCurve::InOutSine);
        visibility_animation_->setStartValue(0);
        visibility_animation_->setEndValue(1);

        connect(teleportPushButton, SIGNAL(clicked()), this, SLOT(Teleport()));
        connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(Cancel()));
        connect(visibility_animation_, SIGNAL(finished()), SLOT(AnimationsFinished()));
        
        hide();
        scene->addItem(this);
            
        connect(scene, SIGNAL(sceneRectChanged(const QRectF&)), SLOT(SceneRectChanged(const QRectF&)));
    }

    // Public

    void TeleportWidget::SetMapBlocks(const QList<ProtocolUtilities::MapBlock> &map_blocks)
    {
        regionComboBox->clear();
        if (map_blocks.size() > 0)
            foreach (ProtocolUtilities::MapBlock block, map_blocks)
                regionComboBox->addItem(QString::fromStdString(block.regionName));
        else
            regionComboBox->setDisabled(true);
    }

    void TeleportWidget::AnimatedHide()
    {
        visibility_animation_->setDirection(QAbstractAnimation::Backward);
        visibility_animation_->start();        
    }

    // Protected

    void TeleportWidget::showEvent(QShowEvent *show_event)
    {
        if (!scene())
            return;

        qreal padding = 10;
        setPos(scene()->sceneRect().right() - size().width() - padding, panel_->GetContentHeight() + padding);
        setOpacity(0);

        QGraphicsProxyWidget::showEvent(show_event);
        visibility_animation_->setDirection(QAbstractAnimation::Forward);
        visibility_animation_->start();
    }

    void TeleportWidget::AnimationsFinished()
    {
        if (visibility_animation_->direction() == QAbstractAnimation::Backward)
        {
            hide();
            emit Hidden();
        }
    }

    // Private

    void TeleportWidget::SceneRectChanged(const QRectF &scene_rect)
    {
        if (isVisible() && scene())
        {        
            qreal padding = 10;
            setPos(scene()->sceneRect().right() - size().width() - padding, panel_->GetContentHeight() + padding);
        }
    }

    void TeleportWidget::Teleport()
    {          
        QString start_location = regionLineEdit->text();
        if (start_location.isEmpty())
            start_location = regionComboBox->currentText();

        if (start_location.isEmpty())
        {
            return;
        }

        emit StartTeleport(start_location);
        AnimatedHide();
    }

    void TeleportWidget::Cancel()
    {
        emit CancelClicked();
        AnimatedHide();
    }
}
