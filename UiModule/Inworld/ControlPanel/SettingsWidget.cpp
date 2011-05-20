//$ HEADER_MOD FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
#include "../../UiModule.h"
#include "../../UiSceneService.h"
#include "../InworldSceneController.h"

#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    SettingsWidget::SettingsWidget(QGraphicsScene *scene, UiServices::UiModule *ui_module) : 
        QWidget(),
        ui_module_(ui_module)
    {
        setupUi(this);
		setObjectName("Naali Settings");
        this->setWindowTitle("Naali Settings");

        connect(opacitySlider, SIGNAL(valueChanged(int)), SLOT(OpacitySliderChanged(int)));
        connect(savePushButton, SIGNAL(clicked()), SLOT(SaveSettings()));
        connect(cancelPushButton, SIGNAL(clicked()), SLOT(Canceled()));

        if (ui_module_->GetFramework()->IsEditionless())
            ui_module->GetUiSceneService()->AddWidgetToScene(this, true , false);
        else
            ui_module->GetUiSceneService()->AddWidgetToScene(this, true , true);

        QPushButton *sett_button = new QPushButton("Settings");
        connect(sett_button, SIGNAL(clicked()),SLOT(ToggleVisibility()));
        ui_module->GetInworldSceneController()->AddAnchoredWidgetToScene(sett_button, Qt::TopRightCorner, Qt::Horizontal, 50, true);
    }

    // Public

    void SettingsWidget::AddWidget(QWidget *widget, const QString &tab_name) const
    {
        settingsTabWidget->addTab(widget, tab_name);
    }

    void SettingsWidget::ToggleVisibility()
    {
        if (this->isVisible())
           ui_module_->GetUiSceneService()->HideWidget(this);
       else
           ui_module_->GetUiSceneService()->ShowWidget(this);
    }

    // Private
    void SettingsWidget::OpacitySliderChanged(int new_value)
    {
        opacityValueLabel->setText(QString("%1 %").arg(QString::number(new_value)));
    }

    void SettingsWidget::SaveSettings()
    {
        emit SaveSettingsClicked();
    }

    void SettingsWidget::Canceled()
    {
		ToggleVisibility();
    }
}
