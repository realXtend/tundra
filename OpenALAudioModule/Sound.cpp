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
    
    bool Sound::LoadFromBuffer(Core::u8* data, Core::uint size, Core::uint frequency, bool sixteenbit, bool stereo)
    {
        DeleteBuffer();
        
        ALenum openal_format;
        if (!stereo)
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_MONO8;
            else
                openal_format = AL_FORMAT_MONO16;
        }
        else
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_STEREO8;
            else
                openal_format = AL_FORMAT_STEREO16;
        }
        
        if (!CreateBuffer())
            return false;
            
        alBufferData(handle_, openal_format, data, size, frequency);
        size_ = size; 
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