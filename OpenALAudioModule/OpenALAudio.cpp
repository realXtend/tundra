#include <al.h>
#include <alc.h>
#include <Poco/Logger.h>
#include "StableHeaders.h"
#include "Foundation.h"
#include "OpenALAudio.h"
#include "OpenALAudioModule.h"


using namespace Foundation;

namespace OpenALAudio
{
    Sound::Sound(Foundation::Framework *framework)
    {
        initialized_ = false;
    }

    Sound::~Sound()
    {
        if (initialized_) Uninitialize();
    }

    void Sound::Initialize()
    {
	    ALCcontext *pContext = NULL;
	    ALCdevice *pDevice = NULL;
        ALenum error;
        //Initial listener properties
        ALfloat listenerPos[] = {0.0,0.0,0.0};
        ALfloat listenerVel[] = {0.0,0.0,0.0};
        ALfloat listenerOrient[] = {0.0,0.0,-1.0, 0.0,1.0,0.0};

        numBuffers_ = 0;
        pDevice = alcOpenDevice(NULL);
        
        if (pDevice)
        {
            pContext = alcCreateContext(pDevice, NULL);
            alcMakeContextCurrent(pContext);
        }
        else 
        {
            return;
        }
        
        alGetError(); //Clear error buffer
        
        numBuffers_ = 257; error = AL_OUT_OF_MEMORY;
        while ((error != AL_NO_ERROR) && (numBuffers_ > 1))
        {
            numBuffers_--;
            alGenBuffers(numBuffers_, buffers_); //Get maximum available buffers
            error = alGetError();
        }
        if (numBuffers_ < 1)
        {
            alcMakeContextCurrent(NULL);
            alcDestroyContext(pContext);
            alcCloseDevice(pDevice);
            return;
        }

        //Set initial listener position, velocity and orientation
        alListenerfv(AL_POSITION, listenerPos);
        alListenerfv(AL_VELOCITY, listenerVel);
        alListenerfv(AL_ORIENTATION, listenerOrient);

        initialized_ = true;
    }

    void Sound::Uninitialize()
    {
        ALCcontext *pContext = NULL;
	    ALCdevice *pDevice = NULL;

        alDeleteBuffers(numBuffers_, buffers_);
        pContext = alcGetCurrentContext();
        pDevice = alcGetContextsDevice(pContext);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(pContext);
        alcCloseDevice(pDevice);

        initialized_ = false;
    }

    void Sound::Update()
    {
         //TODO: Update listener position. Find out if listener should be tied to avatar or to camera
    }

    void Sound::Playsound()
    {
    }

    void Sound::PlayVorbis()
    {
    }

    std::string Sound::LogBufferCount()
    {
        std::stringstream conv;

        conv << numBuffers_;
        
        return conv.str();
    }

    int Sound::GetBufferCount()
    {
        return numBuffers_;
    }
}
