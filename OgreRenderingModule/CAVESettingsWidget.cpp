#include "StableHeaders.h"
#include "CAVESettingsWidget.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Framework.h"

namespace OgreRenderer
{

    CAVESettingsWidget::CAVESettingsWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent)
    {
        setupUi(this);

        framework_ = framework;
         boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();


        if (ui_module.get() == 0)
            return;

        ui_module->GetInworldSceneController()->AddSettingsWidget(this, "CAVE");
        QObject::connect(toggle_CAVE, SIGNAL(toggled(bool)),this, SLOT(CAVEButtonToggled(bool)));
    }


    void CAVESettingsWidget::CAVEButtonToggled(bool v)
    {
        emit ToggleCAVE(v);
    }

}