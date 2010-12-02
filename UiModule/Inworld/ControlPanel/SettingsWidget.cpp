//$ HEADER_MOD FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
#include "Inworld/ControlPanelManager.h"

/*
#include "UiServiceInterface.h"
#include "Inworld/InworldSceneController.h"*/

#include <QPropertyAnimation>
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    SettingsWidget::SettingsWidget(QGraphicsScene *scene, ControlPanelManager *control_panel_manager) : 
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        panel_(control_panel_manager)
    {
        setupUi(internal_widget_);
		// $ BEGIN_MOD $
		// To use it as a external widget
        //setWidget(internal_widget_);
		// $ END_MOD $
		internal_widget_->setObjectName("Naali Settings");

        QFont titleFont("facetextrabold", 10, 25, false);
        titleFont.setCapitalization(QFont::AllUppercase);
        titleFont.setStyleStrategy(QFont::PreferAntialias);
        titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1);
        titleLabel->setFont(titleFont);

        setZValue(100);
        visibility_animation_ = new QPropertyAnimation(this, "opacity", this);
        visibility_animation_->setDuration(500);
        visibility_animation_->setEasingCurve(QEasingCurve::InOutSine);
        visibility_animation_->setStartValue(0);
        visibility_animation_->setEndValue(1);

        connect(opacitySlider, SIGNAL(valueChanged(int)), SLOT(OpacitySliderChanged(int)));
        connect(savePushButton, SIGNAL(clicked()), SLOT(SaveSettings()));
        connect(cancelPushButton, SIGNAL(clicked()), SLOT(Canceled()));
        //connect(visibility_animation_, SIGNAL(finished()), SLOT(AnimationsFinished()));
        connect(scene, SIGNAL(sceneRectChanged(const QRectF&)), SLOT(SceneRectChanged(const QRectF&)));

		//$ BEGIN_MOD $
        //scene->addItem(this);
        //hide();
		// $ END_MOD $
    }

    // Public

    void SettingsWidget::AddWidget(QWidget *widget, const QString &tab_name) const
    {
        settingsTabWidget->addTab(widget, tab_name);
    }

    void SettingsWidget::AnimatedHide()
    {
		//$ BEGIN_MOD $
        //scene->addItem(this);
        internal_widget_->hide();
		// $ END_MOD $
        /*visibility_animation_->setDirection(QAbstractAnimation::Backward);
        visibility_animation_->start();*/
    }

    // Protected

    void SettingsWidget::showEvent(QShowEvent *show_event)
    {
        if (!scene())
            return;
        if (!widget())
            return;

		//$ BEGIN_MOD $
		internal_widget_->show();
		// $ END_MOD $

		/*
        qreal padding = 10;
        setPos(scene()->sceneRect().right() - widget()->size().width() - padding, panel_->GetContentHeight() + padding);
        setOpacity(0);

        QGraphicsProxyWidget::showEvent(show_event);
        visibility_animation_->setDirection(QAbstractAnimation::Forward);
        visibility_animation_->start();*/
    }

    void SettingsWidget::AnimationsFinished()
    {
        if (visibility_animation_->direction() == QAbstractAnimation::Backward)
        {
            //hide();
            emit Hidden();
        }
    }

    // Private

    void SettingsWidget::SceneRectChanged(const QRectF &scene_rect)
    {
        if (scene() && widget())
        {        
            qreal padding = 10;
            setPos(scene()->sceneRect().right() - widget()->size().width() - padding, panel_->GetContentHeight() + padding);
        }
    }

    void SettingsWidget::OpacitySliderChanged(int new_value)
    {
        opacityValueLabel->setText(QString("%1 %").arg(QString::number(new_value)));
    }

    void SettingsWidget::SaveSettings()
    {
        // Emit extra params for InworldSceneManager
        int new_animation_speed = showAnimationSpinBox->value();
        UNREFERENCED_PARAM(new_animation_speed);
        if (animationEnabledCheckBox->checkState() == Qt::Unchecked)
            emit NewUserInterfaceSettingsApplied(opacitySlider->value(), 0);
        else
            emit NewUserInterfaceSettingsApplied(opacitySlider->value(), showAnimationSpinBox->value());
        emit SaveSettingsClicked();
        
        AnimatedHide();
    }

    void SettingsWidget::Canceled()
    {
        emit CancelClicked();
        AnimatedHide();
    }
}
