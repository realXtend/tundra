// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudioModule_h
#define incl_OpenALAudioModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "OpenALAudio.h"

namespace Foundation
{
    class Framework;
}

namespace OpenALAudio
{
    class Sound;
    typedef boost::shared_ptr<Sound> SoundPtr;

    //! interface for modules
    class MODULE_API OpenALAudioModule : public Foundation::ModuleInterfaceImpl
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
		virtual void Update();

		MODULE_LOGGING_FUNCTIONS;

		//! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Sound;

    private:
		OpenALAudio::SoundPtr sound_;
    };
}

#endif
