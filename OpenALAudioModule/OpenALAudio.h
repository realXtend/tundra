// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudioModule_OpenALAudio_h
#define incl_OpenALAudioModule_OpenALAudio_h

#include "SoundServiceInterface.h"
#include <boost/shared_ptr.hpp>

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

		void Update(); //Per frame updating. Usually nothing is done here, but it is here in case it is needed.

		virtual void Playsound(); //! todo add functionality

		virtual void PlayVorbis(); //! todo add functionality
    };

    typedef boost::shared_ptr<Sound> SoundPtr;
}

#endif