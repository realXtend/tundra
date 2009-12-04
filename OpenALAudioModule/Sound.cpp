// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Sound.h"

namespace OpenALAudio
{
    
    Sound::Sound() : handle_(0), datasize_(0)
    {
    }
    
    Sound::~Sound()
    {
        if (handle_)
        {
            alDeleteBuffers(1, &handle_);
            handle_ = 0;
        }
    }
}