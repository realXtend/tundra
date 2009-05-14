// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudioModule_OpenALAudio_h
#define incl_OpenALAudioModule_OpenALAudio_h

#include "SoundServiceInterface.h"
#include <boost/shared_ptr.hpp>
#include <al.h>

namespace Foundation
{
    class Framework;
}

namespace OpenALAudio
{
    class Sound : public Foundation::SoundServiceInterface
    {
	public:
        Sound(Foundation::Framework *framework);
		virtual ~Sound();

		void Initialize();

        void Uninitialize();

		void Update(); //Per frame updating. Updates the listener position

		virtual void Playsound(); //! todo add functionality

		virtual void PlayVorbis(); //! todo add functionality

        std::string LogBufferCount(); //Returns initialized sound buffer count for loggin
        int GetBufferCount(); //Returns initialized sound buffer count

    private:
        bool initialized_;
        ALuint buffers_[256];
        ALint numBuffers_;
    };

    typedef boost::shared_ptr<Sound> SoundPtr;
}

#endif