// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Sound.h"
#include "OpenALAudioModule.h"

namespace OpenALAudio
{            
    Sound::Sound(const std::string& name) : 
        name_(name),
        handle_(0), 
        size_(0), 
        age_(0.0)
    {
    }
    
    Sound::~Sound()
    {
        DeleteBuffer();
    }
    
    bool Sound::LoadFromBuffer(const ISoundService::SoundBuffer& buffer)
    {
        DeleteBuffer();
        
        ALenum openal_format;
        if (!buffer.stereo_)
        {
            if (!buffer.sixteenbit_)
                openal_format = AL_FORMAT_MONO8;
            else
                openal_format = AL_FORMAT_MONO16;
        }
        else
        {
            if (!buffer.sixteenbit_)
                openal_format = AL_FORMAT_STEREO8;
            else
                openal_format = AL_FORMAT_STEREO16;
        }
        
        if (!CreateBuffer())
            return false;
        
        alGetError();
        alBufferData(handle_, openal_format, (u8*)&buffer.data_[0], buffer.data_.size(), buffer.frequency_);
        ALenum error = alGetError();
        if (error != AL_NONE)
        {
            OpenALAudioModule::LogError("Could not set OpenAL sound buffer data: " + ToString<int>(error));
            return false;
        }
        size_ = buffer.data_.size();
        return true;
    }
    
    bool Sound::CreateBuffer()
    {    
        if (!handle_)
            alGenBuffers(1, &handle_);
        
        if (!handle_)
        {
            OpenALAudioModule::LogError("Could not create OpenAL sound buffer");
            return false;
        } 
        
        return true;
    }
        
    void Sound::DeleteBuffer()
    {
        if (handle_)
        {
            alDeleteBuffers(1, &handle_);
            handle_ = 0;
            size_ = 0;
        }
    } 
}