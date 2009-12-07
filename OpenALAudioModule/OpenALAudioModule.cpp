// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenALAudioModule.h"
#include "SoundSystem.h"
#include "EC_AttachedSound.h"

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
		using namespace OpenALAudio;

		LogInfo("Module " + Name() + " loaded.");
		DECLARE_MODULE_EC(EC_AttachedSound);
	}

    //Virtual
    void OpenALAudioModule::Unload()
	{
		LogInfo("Module " + Name() + " unloaded.");
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
    }

    void OpenALAudioModule::PostInitialize()
	{
	}

    void OpenALAudioModule::Uninitialize()
	{
        framework_->GetServiceManager()->UnregisterService(soundsystem_);
		soundsystem_.reset();

		LogInfo("Module " + Name() + " uninitialized.");
	}

    void OpenALAudioModule::Update(Core::f64 frametime)
	{
        {
            PROFILE(OpenALAudioModule_Update);
            if (soundsystem_)
                soundsystem_->Update(frametime);
        }
        RESETPROFILER;
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
