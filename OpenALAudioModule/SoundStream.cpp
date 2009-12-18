// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundStream.h"

// Debug prints only
#include "OpenALAudioModule.h"

namespace OpenALAudio
{
    SoundStream::SoundStream(std::string stream_name, uint frequency, int sample_width, bool stereo)
        : name_(stream_name),
          frequency_(frequency),
          sample_width_(sample_width),
          stereo_(stereo),
          current_buffer_(0),
          buffer_counter_(0),
          next_free_data_queue_index_(0),
          free_data_queque_index_count_(DATA_QUEUE_SIZE)
    {
        for (int i = 0; i < DATA_QUEUE_SIZE; i++)
        {
            data_queque_[i] = 0;
        }

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

        alGenBuffers(MAX_BUFFER_COUNT, buffers_);
        alGenSources(1, &source_);

        alSourcef(source_, AL_GAIN, 1.0f);
     //   alSourcef(source_, AL_ROLLOFF_FACTOR, 0.0);

        // Add empty buffers to source
        //for (int buffer_count = 0; buffer_count<BUFFER_COUNT; ++buffer_count)
        //{
        //    u8 *temp = new u8[10];
        //    for (int char_index = 0; char_index<10; ++char_index)
        //        temp[char_index] = 0;
        //    alBufferData(buffers_[buffer_count], format_, temp, 10, frequency_);
        //}
        //alSourceQueueBuffers(source_, BUFFER_COUNT, buffers_);
        //Play();
    }

    SoundStream::~SoundStream()
    {
        Release();
    }

    void SoundStream::Release()
    {
        alSourceStop(source_);
        alDeleteSources(1, &source_);
        alDeleteBuffers(MAX_BUFFER_COUNT, buffers_);
        for (int i = 0; i < DATA_QUEUE_SIZE; i++)
        {
            if (data_queque_[i] != 0)
            {
                delete [] data_queque_[i];
                data_queque_[i] = 0;
            }
        }
    }

    void SoundStream::Play()
    {
        alSourcePlay(source_);
        OpenALAudioModule::LogDebug(">> Stream playback started");
    }

    void SoundStream::AddData(u8 *data, uint size)
    {
        ALuint buffer = GetBufferWithData(data, size);
        if (buffer == 0)
            return;

        // Queue buffer back to sources playlist
        alSourceQueueBuffers(source_, 1, &buffer);

        if (!IsPlaying() && (buffer_counter_ > MAX_BUFFER_COUNT*0.5))
            Play();
    }

    ALuint SoundStream::GetBufferWithData(u8* data, uint size)
    {
        if (buffer_counter_ < MAX_BUFFER_COUNT)
        {
            u8* data_slot = StoreData(data, size);
            if (!data_slot)
                return 0;

            ALuint buffer = buffers_[buffer_counter_++];
            alBufferData(buffer, format_, data_slot, size, frequency_);
            alSourceQueueBuffers(source_, 1, &buffer);
            OpenALAudioModule::LogDebug("Added buffer " + boost::lexical_cast<std::string>(buffer_counter_+1) + "/" + boost::lexical_cast<std::string>(MAX_BUFFER_COUNT) + " to source");
            return buffer;
        }
        else
        {
            ALint empty_buffer_count = 0;
            alGetSourcei(source_, AL_BUFFERS_PROCESSED, &empty_buffer_count);
            if (empty_buffer_count == 0)
                return 0;
            ALuint buffer;
            alSourceUnqueueBuffers(source_, 1, &buffer);
                
            if (alGetError() == AL_INVALID_VALUE)
            {
                OpenALAudioModule::LogDebug("Could not pull empty buffer from source!");
                return 0;
            }
            else
                free_data_queque_index_count_++; // todo: call FreeData etc. method

            u8* data_slot = StoreData(data, size);
            if (!data_slot)
                return 0;
            if (free_data_queque_index_count_ >= DATA_QUEUE_SIZE)
                assert(false); // Should never happen

            alBufferData(buffer, format_, data_slot, size, frequency_);
            return buffer;
        }
    }
    
    u8* SoundStream::StoreData(u8 *data, uint size)
    {
        if ( free_data_queque_index_count_ == 0)
            return 0;
        
        u8* data_slot = data_queque_[next_free_data_queue_index_];
        if (data_slot)
        {
            delete [] data_slot;
            data_slot = 0;
        }
        data_slot = new u8[size];
        memcpy(data_slot, data, size);
        next_free_data_queue_index_ = (next_free_data_queue_index_ + 1 ) % DATA_QUEUE_SIZE;
        free_data_queque_index_count_--;
        return data_slot;
    }

    void SoundStream::ReleaseData()
    {

    }

    bool SoundStream::IsPlaying()
    {
        ALenum state;
        alGetSourcei(source_, AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }

    void SoundStream::SetPosition(const Vector3df &position)
    {
        if (source_)
        {
            alSourcei(source_, AL_SOURCE_RELATIVE, AL_FALSE);
            ALfloat sound_pos[] = { position.x, position.y, position.z };
            alSourcefv(source_, AL_POSITION, sound_pos);
        }
    }
}
