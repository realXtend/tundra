// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SettingsWidget.h"

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    SettingsWidget::SettingsWidget()
        : QWidget()
    {
        settings_ui_.setupUi(this);
        connect(settings_ui_.opacitySlider, SIGNAL( valueChanged(int) ),
                this, SLOT( OpacitySliderChanged(int) ));
        connect(settings_ui_.applyPushButton, SIGNAL( clicked() ),
                this, SLOT( ApplySettings() ));
    }

    SettingsWidget::~SettingsWidget()
    {
    }

    void SettingsWidget::AddWidget(QWidget *widget, const QString &tab_name)
    {
        settings_ui_.settingsTabWidget->addTab(widget, tab_name);
    }

    void SettingsWidget::OpacitySliderChanged(int new_value)
    {
        settings_ui_.opacityValueLabel->setText(QString("%1 %").arg(QString::number(new_value)));
    }

    void SettingsWidget::ApplySettings()
    {
        int new_opacity = settings_ui_.opacitySlider->value();
        int new_animation_speed = settings_ui_.showAnimationSpinBox->value();
        if (settings_ui_.animationEnabledCheckBox->checkState() == Qt::Unchecked)
            new_animation_speed = 0; // Equals as disabled in the proxy layer
        emit NewUserInterfaceSettingsApplied(new_opacity, new_animation_speed);
    }
}