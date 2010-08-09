// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RendererSettings.h"
#include "OgreRenderingModule.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "Inworld/InworldSceneController.h"
#include "Framework.h"

#include <QUiLoader>
#include <QFile>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "UiModule.h"

namespace OgreRenderer
{

    RendererSettings::RendererSettings(Foundation::Framework* framework) :
        framework_(framework),
        settings_widget_(0)
    {
        InitWindow();
    }
    
    RendererSettings::~RendererSettings()
    {
        SAFE_DELETE(settings_widget_);
    }

    void RendererSettings::InitWindow()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();

        // If this occurs, we're most probably operating in headless mode.
        if (ui_module.get() == 0)
            return;

        QUiLoader loader;
        QFile file("./data/ui/renderersettings.ui");

        if (!file.exists())
        {
            OgreRenderingModule::LogError("Cannot find renderer settings .ui file.");
            return;
        }

        settings_widget_ = loader.load(&file); 
        if (!settings_widget_)
            return;

        ui_module->GetInworldSceneController()->AddSettingsWidget(settings_widget_, "Rendering");

        QDoubleSpinBox* spin = settings_widget_->findChild<QDoubleSpinBox*>("spinbox_viewdistance");
        boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService<Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!spin || !renderer)
            return;
        spin->setValue(renderer->GetViewDistance());
        QCheckBox* cbox = settings_widget_->findChild<QCheckBox*>("fullscreen_toggle");
        if(cbox)
        {
            cbox->setChecked(renderer->IsFullScreen());
            QObject::connect(cbox, SIGNAL(toggled(bool)), this, SLOT(SetFullScreenMode(bool)));
        }
        QObject::connect(spin, SIGNAL(valueChanged(double)), this, SLOT(ViewDistanceChanged(double)));
    }
    
    void RendererSettings::ViewDistanceChanged(double value)
    {
        boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService<Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;   
        renderer->SetViewDistance(value);      
    }        

    void RendererSettings::SetFullScreenMode(bool value)
    {
         boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService<Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;   
        renderer->SetFullScreen(value);   
    }
}