

#ifndef __incl_Interfaces_SoundSystemInterface_h__
#define __incl_Interfaces_SoundSystemInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class SoundSystemInterface : public ServiceInterface
    {
    public:
        SoundSystemInterface() {}
        virtual ~SoundSystemInterface() {}

        //! Plays non-3D sound
        virtual void playsound() = 0;

        //! plays ogg vorbis (music) file
        virtual void playVorbis() = 0;
    };
}

#endif // __incl_Interfaces_SoundSystemInterface_h__
