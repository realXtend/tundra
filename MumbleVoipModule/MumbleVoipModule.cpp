// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MumbleVoipModule.h"
#include "ModuleManager.h"
#include "WorldLogicInterface.h"
#include "Entity.h"
#include "ConsoleCommandUtils.h"
#include "EventManager.h"
#include "Provider.h"
#include "MumbleLibrary.h"
#include "SettingsWidget.h"
#include "UiServiceInterface.h"
#include "ConsoleAPI.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    std::string MumbleVoipModule::module_name_ = "MumbleVoip";

    MumbleVoipModule::MumbleVoipModule()
        : IModule(module_name_),
          in_world_voice_provider_(0),
          settings_widget_(0)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(in_world_voice_provider_);
        SAFE_DELETE(settings_widget_);
    }

    void MumbleVoipModule::Load()
    {
    }

    void MumbleVoipModule::Unload()
    {
    }

    void MumbleVoipModule::Initialize()
    {
    }

    void MumbleVoipModule::PostInitialize()
    {
        in_world_voice_provider_ = new Provider(framework_, &settings_);
        in_world_voice_provider_->PostInitialize();

        InitializeConsoleCommands();
        
        event_category_framework_ = framework_->GetEventManager()->QueryEventCategory("Framework");
        if (event_category_framework_ == 0)
            LogError("Unable to find event category for Framework");
       SetupSettingsWidget();
    }

    void MumbleVoipModule::Uninitialize()
    {
        SAFE_DELETE(in_world_voice_provider_);

        MumbleLib::MumbleLibrary::Stop();
    }

    void MumbleVoipModule::Update(f64 frametime)
    {   
        if (in_world_voice_provider_)
            in_world_voice_provider_->Update(frametime);
    }

    bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        if (in_world_voice_provider_)
            in_world_voice_provider_->HandleEvent(category_id, event_id, data);

        return false;
    }

    void MumbleVoipModule::InitializeConsoleCommands()
    {
        /// \note Do we still wan't to use console commands with the libmumbleclient implementation?
    }

    void MumbleVoipModule::SetupSettingsWidget()
        {
            UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
            if (!ui)
                return;

            settings_widget_ = new SettingsWidget(in_world_voice_provider_, &settings_);

            //QUiLoader loader;
            //QFile file("./data/ui/soundsettings.ui");

            //if (!file.exists())
            //{
            //    OpenALAudioModule::LogError("Cannot find sound settings .ui file.");
            //    return;
            //}

            //settings_widget_ = loader.load(&file);
            //if (!settings_widget_)
            //    return;

            ui->AddSettingsWidget(settings_widget_, "Voice");
        }
} // end of namespace: MumbleVoip

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace MumbleVoip;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(MumbleVoipModule)
POCO_END_MANIFEST
