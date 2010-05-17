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

    void TeleportWidget::SetRegionNames(const std::vector<std::string> &region_names)
    {
        regionComboBox->clear();
        if (region_names.size() > 0)
        {
            for(uint i = 0; i < region_names.size(); i++)
            {                
                regionComboBox->addItem(QString::fromStdString(region_names[i]));
            }
        }
        else
        {
            regionComboBox->setDisabled(true);
        }
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
        QString start_location = regionComboBox->currentText();
        if (start_location.isNull())
            start_location = regionLineEdit->text();


        if (start_location.isNull())
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
