// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundStream.h"

// Debug prints only
#include "OpenALAudioModule.h"

namespace OpenALAudio
{
    SoundStream::SoundStream(std::string stream_name, Core::uint frequency, int sample_width, bool stereo)
        : name_(stream_name),
          frequency_(frequency),
          sample_width_(sample_width),
          stereo_(stereo),
          current_buffer_(0),
          counter_(0)

    {
        switch (sample_width_)
        {
            case 8:
                if (stereo)
                    format_ = AL_FORMAT_STEREO8;
                else
                    format_ = AL_FORMAT_MONO8;
                break;
            case 16:
                if (stereo)
                    format_ = AL_FORMAT_STEREO16;
                else
                    format_ = AL_FORMAT_MONO16;
                break;
        }

        alGenBuffers(BUFFER_COUNT, buffers_);
        alGenSources(1, &source_);

        alSourcef(source_, AL_GAIN, 1.0f);
        alSourcef(source_, AL_ROLLOFF_FACTOR, 0.0);
    }

    SoundStream::~SoundStream()
    {
        Release();
    }

    void SoundStream::Release()
    {
        alSourceStop(source_);
        alDeleteSources(1, &source_);
        alDeleteBuffers(BUFFER_COUNT, buffers_);
    }

    void SoundStream::Play()
    {
        alSourcePlay(source_);
        OpenALAudioModule::LogDebug(">> Stream playback started");
    }

    void SoundStream::AddBuffer(Core::u8 *data, Core::uint size)
    {
        alBufferData(buffers_[counter_], format_, data, size, frequency_);
        alSourceQueueBuffers(source_, 1, &buffers_[counter_]);
        OpenALAudioModule::LogDebug("Added buffer " + boost::lexical_cast<std::string>(counter_+1) + "/" + boost::lexical_cast<std::string>(BUFFER_COUNT) + " to source");
        counter_++;
    }

    void SoundStream::FillBuffer(Core::u8 *data, Core::uint size)
    {
        if (counter_ < BUFFER_COUNT)
        {
            AddBuffer(data, size);
            int test = BUFFER_COUNT - counter_;
            // Start playback when after some buffers have already been filled
            if (!IsPlaying() && BUFFER_COUNT - counter_ < BUFFER_COUNT / 2)
                Play();
        }
        else
        {
            // Unqueue one buffer
            ALuint buffer;
            alSourceUnqueueBuffers(source_, 1, &buffer);
            if (alGetError() == AL_INVALID_VALUE)
            {
                OpenALAudioModule::LogDebug("Could not pull empty buffer from source!");
                return;
            }

            // Fill buffer with data
            alBufferData(buffer, format_, data, size, frequency_);

            // Queue buffer back to sources playlist
            alSourceQueueBuffers(source_, 1, &buffer);
        
            if (!IsPlaying())
                Play();
        }
    }

    bool SoundStream::IsPlaying()
    {
        ALenum state;
        alGetSourcei(source_, AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }


}