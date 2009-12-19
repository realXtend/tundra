// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundStream.h"

// Debug prints only
#include "OpenALAudioModule.h"
#include <QFile>

namespace OpenALAudio
{
    SoundStream::SoundStream(std::string stream_name, uint frequency, int sample_width, bool stereo)
        : name_(stream_name),
          frequency_(frequency),
          sample_width_(sample_width),
          stereo_(stereo),
          current_buffer_(0)
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

        alGenSources(1, &source_);
        alSourcef(source_, AL_GAIN, 1.0f);
        alSourcef(source_, AL_ROLLOFF_FACTOR, 0.0); // TODO: Check this for spatial playback 

        alGenBuffers(MAX_BUFFER_COUNT, buffers_);
        for (int i = 0; i < MAX_BUFFER_COUNT; i++)
        {
            ALuint buffer_handle = buffers_[i];
            playback_buffers_[buffer_handle] = 0;
            alBufferData(buffer_handle, format_, playback_buffers_[buffer_handle], 0, frequency_);
            alSourceQueueBuffers(source_, 1, &buffer_handle); 
        }
        Play();
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
        for (int i = 0; i < MAX_BUFFER_COUNT; i++)
        {
            delete [] playback_buffers_[i];
            playback_buffers_[i] = 0;
        }
        for (std::vector<u8*>::iterator i = data_queue_.begin(); i != data_queue_.end(); ++i)
        {
            u8* data = *i;
            delete [] data;
            data = 0;
        }
        data_queue_.clear();
        data_queue_packet_sizes_.clear();
    }

    void SoundStream::Play()
    {
        alSourcePlay(source_);
        OpenALAudioModule::LogDebug(">> Stream playback started");
    }

    int SoundStream::GetReceivedAudioDataLengthMs()
    {
        u32 byte_count = 0;
        for (std::vector<u32>::iterator i = data_queue_packet_sizes_.begin(); i != data_queue_packet_sizes_.end(); ++i)
        {
            u32 size = *i;
            byte_count += size;
        }

        return byte_count*1000*8/sample_width_/frequency_;
    }

    void SoundStream::StoreToQueue(u8* data, int size)
    {
        u8* local_copy = new u8[size]; // RESERVE MEMORY
        memcpy(local_copy, data, size);
        data_queue_.push_back(local_copy);
        data_queue_packet_sizes_.push_back(size);
    }

    ALint SoundStream::FillBufferFromQueue(ALint buffer_handle)
    {
        int total_queue_size = 0;
        for (std::vector<u32>::iterator i = data_queue_packet_sizes_.begin(); i != data_queue_packet_sizes_.end(); ++i)
        {
            u32 size = *i;
            total_queue_size += size;
        }
        u8* local_copy = new u8[total_queue_size]; // RESERVE MEMORY
        u32 offset = 0;
        for (int i = 0; i < data_queue_.size(); i++)
        {
            u32 size = data_queue_packet_sizes_[i];
            memcpy(local_copy + offset, data_queue_[i], size);
            delete [] data_queue_[i]; // FREE MEMORY
            offset += size;
        }
        data_queue_.clear();
        data_queue_packet_sizes_.clear();

        // clear previously playback buffer 
        u8* playback_buffer  = playback_buffers_[buffer_handle];
        if (playback_buffer)
            delete [] playback_buffer; // FREE MEMORY

        playback_buffers_[buffer_handle] = local_copy;

        alBufferData(buffer_handle, format_, playback_buffers_[buffer_handle], total_queue_size, frequency_);
        return buffer_handle;
    }

    void SoundStream::AddData(u8 *data, uint size)
    {
        // TEST
        //QFile file("audio.raw");
        //file.open(QIODevice::WriteOnly | QIODevice::OpenModeFlag::Append);
        //file.write((char*)data, size);
        //file.close();

        if (!add_data_mutex_.tryLock())
            return;
        int max_buffer_length_ms = 1000;

        ALint empty_buffer_count = 0;
        alGetSourcei(source_, AL_BUFFERS_PROCESSED, &empty_buffer_count);
        if (empty_buffer_count == 0 && GetReceivedAudioDataLengthMs() > max_buffer_length_ms)
        {
//            OpenALAudioModule::LogDebug("Drop audio packet, no buffers for playback.");
            add_data_mutex_.unlock();
            return;
        }

        StoreToQueue(data, size);

        if (empty_buffer_count == 0 || GetReceivedAudioDataLengthMs() < max_buffer_length_ms / 2)
        {
            // we do not want to fill OpenAL buffer even if we have one available
            add_data_mutex_.unlock();
            return;
        }

        ALuint buffer_handle;
        alSourceUnqueueBuffers(source_, 1, &buffer_handle);
        if (alGetError() == AL_INVALID_VALUE)
        {
            OpenALAudioModule::LogDebug("Could not pull empty buffer from source!");
            add_data_mutex_.unlock();
            return;
        }

        if (!FillBufferFromQueue(buffer_handle))
            return;

        // Queue buffer back to sources playlist
        alSourceQueueBuffers(source_, 1, &buffer_handle);

        if (!IsPlaying())
            Play();

        add_data_mutex_.unlock();
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
