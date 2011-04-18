// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MumbleVoipModule.h"
#include "LinkPlugin.h"
#include "Provider.h"
#include "MumbleLibrary.h"
#include "SettingsWidget.h"
#include "Settings.h"

#include "Entity.h"
#include "EC_Placeable.h"
#include "EC_VoiceChannel.h"

#include "UiAPI.h"
#include "UiMainWindow.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    MumbleVoipModule::MumbleVoipModule()
    :IModule("MumbleVoip"),
    link_plugin_(0),
    in_world_voice_provider_(0),
    time_from_last_update_ms_(0),
    settings_widget_(0),
    settings_(0)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(link_plugin_);
        SAFE_DELETE(in_world_voice_provider_);
        SAFE_DELETE(settings_widget_);
        SAFE_DELETE(settings_);
    }

    void MumbleVoipModule::Load()
    {
        DECLARE_MODULE_EC(EC_VoiceChannel);
        link_plugin_ = new LinkPlugin();
    }

    void MumbleVoipModule::Unload()
    {
    }

    void MumbleVoipModule::Initialize()
    {
    }

    void MumbleVoipModule::PostInitialize()
    {
        settings_ = new Settings(framework_);
        in_world_voice_provider_ = new Provider(framework_, settings_);
        in_world_voice_provider_->PostInitialize();

        SetupSettingsWidget(); 
    }

    void MumbleVoipModule::Uninitialize()
    {
        SAFE_DELETE(link_plugin_);
        SAFE_DELETE(in_world_voice_provider_);
        MumbleLib::MumbleLibrary::Stop();
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
        if (link_plugin_ && link_plugin_->IsRunning())
            UpdateLinkPlugin(frametime);
        if (in_world_voice_provider_)
            in_world_voice_provider_->Update(frametime);
    }

    void MumbleVoipModule::UpdateLinkPlugin(f64 frametime)
    {
        /// \todo inspect what this link plugin actually is, seems like it can be removed
        if (!link_plugin_)
            return;

        time_from_last_update_ms_ += 1000*frametime;
        if (time_from_last_update_ms_ < LINK_PLUGIN_UPDATE_INTERVAL_MS_)
            return;
        time_from_last_update_ms_ = 0;

        /*
        Vector3df top_vector = Vector3df::UNIT_Z, position, direction;
        if (GetAvatarPosition(position, direction))
            link_plugin_->SetAvatarPosition(position, direction, top_vector);

        if (use_camera_position_)
            if (GetCameraPosition(position, direction))
                link_plugin_->SetCameraPosition(position, direction, top_vector);
        else
            if (GetAvatarPosition(position, direction))
                link_plugin_->SetCameraPosition(position, direction, top_vector);
        */
        link_plugin_->SendData();
    }

    void MumbleVoipModule::SetupSettingsWidget()
    {
        if (GetFramework()->IsHeadless())
            return;
        settings_widget_ = new SettingsWidget(in_world_voice_provider_, settings_);
        settings_widget_->setParent(framework_->Ui()->MainWindow());
        settings_widget_->setWindowFlags(Qt::Tool);
        settings_widget_->resize(1,1);
    }

    void MumbleVoipModule::ToggleSettingsWidget()
    {
        if (settings_widget_)
            settings_widget_->setVisible(!settings_widget_->isVisible());
    }

} // end of namespace: MumbleVoip

void SetProfiler(Profiler *profiler)
{
    ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
    __declspec(dllexport) void TundraPluginMain(Framework *fw)
    {
        IModule *module = new MumbleVoip::MumbleVoipModule();
        fw->GetModuleManager()->DeclareStaticModule(module);
    }
}
