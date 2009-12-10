// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_OpenALAudioModule_h
#define incl_OpenALAudio_OpenALAudioModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "OpenALAudioModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace OpenALAudio
{
    class SoundSystem;
    typedef boost::shared_ptr<SoundSystem> SoundSystemPtr;

    //! interface for modules
    class OPENAL_MODULE_API OpenALAudioModule : public Foundation::ModuleInterfaceImpl
	{
    public:
		OpenALAudioModule();
		virtual ~OpenALAudioModule();

		virtual void Load();
		virtual void Unload();
		virtual void PreInitialize();
		virtual void Initialize();
		virtual void Uninitialize();
		virtual void PostInitialize();
		virtual void Update(Core::f64 frametime);

		MODULE_LOGGING_FUNCTIONS;

		//! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Sound;
      
        bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);
                
    private:
		SoundSystemPtr soundsystem_;
		
		Core::event_category_id_t task_event_category_;
		Core::event_category_id_t asset_event_category_;
    };
}

#endif
