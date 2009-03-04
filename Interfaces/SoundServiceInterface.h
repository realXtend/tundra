// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_SoundServiceInterface_h__
#define __incl_Interfaces_SoundServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class SoundServiceInterface : public ServiceInterface
    {
    public:
        SoundServiceInterface() {}
        virtual ~SoundServiceInterface() {}

        //! Plays non-3D sound
        virtual void playsound() = 0;

        //! plays ogg vorbis (music) file
        virtual void playVorbis() = 0;
    };
}

#endif

