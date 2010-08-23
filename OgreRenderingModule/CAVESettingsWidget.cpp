#include "StableHeaders.h"
#include "CAVESettingsWidget.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Framework.h"
#include <Ogre.h>
#include <QDebug>

namespace OgreRenderer
{

    CAVESettingsWidget::CAVESettingsWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent),
        settings_dialog_(this),
        next_view_num_(0),
        view_prefix_("View")
    {
        setupUi(this);

        framework_ = framework;
         boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();


        if (ui_module.get() == 0)
            return;

        ui_module->GetInworldSceneController()->AddSettingsWidget(this, "CAVE");
        QObject::connect(toggle_CAVE, SIGNAL(toggled(bool)),this, SLOT(CAVEButtonToggled(bool)));
        QObject::connect(addView, SIGNAL(clicked(bool)),this,SLOT(AddNewCAVEView()));
        
    }

    void CAVESettingsWidget::AddNewCAVEView()
    {
        if(settings_dialog_.exec() == QDialog::Accepted)
        {
            Ogre::Vector3 bl, br, tl, eye;
            settings_dialog_.getCaveProjectionSettings(eye,bl,tl,br);
            QString view_name = view_prefix_;
            view_name += next_view_num_;
            qDebug() << view_name;
            emit NewCAVEViewRequested(view_name, tl ,bl, br, eye);
            next_view_num_++;

        }
    }


    void CAVESettingsWidget::CAVEButtonToggled(bool v)
    {
        emit ToggleCAVE(v);
    }

}