// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SoundServiceInterface_h
#define incl_Interfaces_SoundServiceInterface_h

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

