// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenALAudioModule.h"
#include "SoundSystem.h"
#include "SoundSettings.h"
#include "Framework.h"
#include "ServiceManager.h"
#include "EventManager.h"

namespace OpenALAudio
{
    OpenALAudioModule::OpenALAudioModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    OpenALAudioModule::~OpenALAudioModule()
    {
    }

    //Virtual
    void OpenALAudioModule::Load()
    {
    }

    //Virtual
    void OpenALAudioModule::Unload()
    {
    }

    void OpenALAudioModule::PreInitialize()
    {
    }

    //Virtual
    void OpenALAudioModule::Initialize()
    {
        soundsystem_ = SoundSystemPtr(new SoundSystem(framework_));
        if (!soundsystem_->IsInitialized())
            return;
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Sound, soundsystem_);

        // Sound settings depends on the sound service, so init it last
        soundsettings_ = SoundSettingsPtr(new SoundSettings(framework_));
    }

    void OpenALAudioModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        asset_event_category_ = event_manager->QueryEventCategory("Asset");
        task_event_category_ = event_manager->QueryEventCategory("Task");
    }

    void OpenALAudioModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(soundsystem_);
        soundsystem_.reset();
    }

    void OpenALAudioModule::Update(f64 frametime)
    {
        {
            PROFILE(OpenALAudioModule_Update);
            if (soundsystem_)
                soundsystem_->Update(frametime);
        }
        RESETPROFILER;
    }

    bool OpenALAudioModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (category_id == asset_event_category_)
        {
            if (soundsystem_)
                return soundsystem_->HandleAssetEvent(event_id, data);
            else return false;
        }
        if (category_id == task_event_category_)
        {
            if (soundsystem_)
                return soundsystem_->HandleTaskEvent(event_id, data);
            else return false;
        }
        return false;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OpenALAudio;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(OpenALAudioModule)
POCO_END_MANIFEST
