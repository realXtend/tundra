// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundChannel.h"

namespace OpenALAudio
{
    SoundChannel::SoundChannel() :
        handle_(0),
        pitch_(1.0f),
        gain_(0.0f),
        positional_(false),
        looped_(false),
        state_(Foundation::SoundServiceInterface::Stopped)
    { 
    }
    
    SoundChannel::~SoundChannel()
    {       
        DeleteSource();
    }
    
    void SoundChannel::Update()
    {
        switch (state_)
        {
            case Foundation::SoundServiceInterface::Playing:
            if (handle_)
            {
                ALint playing;
                alGetSourcei(handle_, AL_SOURCE_STATE, &playing);
                if (playing != AL_PLAYING)
                    state_ = Foundation::SoundServiceInterface::Stopped;
            }
            break;
        }
    }
    
    bool SoundChannel::CreateSource()
    {
        if (!handle_)
            alGenSources(1, &handle_);
        
        return (handle_ != 0);
    }
    
    void SoundChannel::DeleteSource()
    {
        if (handle_)
        {
            alDeleteSources(1, &handle_);
            handle_ = 0;
        }    
    }
}