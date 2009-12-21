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
          test_(0)
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
        alSourcef(source_, AL_REFERENCE_DISTANCE, 10.0);
        alSourcei(source_, AL_LOOPING, AL_FALSE);
        alSourcei(source_, AL_SOURCE_RELATIVE, AL_FALSE);

        alGenBuffers(MAX_BUFFER_COUNT, buffers_);
        for (int i = 0; i < MAX_BUFFER_COUNT; i++)
        {
            const int empty_buffer_fill_size = 100;
            ALuint buffer_handle = buffers_[i];
            playback_buffers_[buffer_handle] = new u8[empty_buffer_fill_size];
            memset(playback_buffers_[buffer_handle], 0, empty_buffer_fill_size);
            alBufferData(buffer_handle, format_, playback_buffers_[buffer_handle], empty_buffer_fill_size, frequency_);
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
        ALint buffers_in_play = 0;
        alGetSourcei(source_, AL_BUFFERS_QUEUED, &buffers_in_play);

        ALint state;
        alGetSourcei(source_, AL_SOURCE_STATE, &state);

        if (buffers_in_play > 0 )
        {
//            return;
        }

        if (state == AL_PLAYING)
            return;

        alGetError();
        alSourcePlay(source_);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR)
        {
            OpenALAudioModule::LogError("Cannot alSourcePlay failed!");
        }
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
        //QFile file("audio-StoreToQueue.raw");
        //file.open(QIODevice::OpenModeFlag::Append);
        //file.write((char*)data, size);
        //file.close();

        u8* local_copy = new u8[size]; // RESERVE MEMORY
        memcpy(local_copy, data, size);
        data_queue_.push_back(local_copy);
        data_queue_packet_sizes_.push_back(size);
        assert(data_queue_.size() == data_queue_packet_sizes_.size());
    }

    ALint SoundStream::FillBufferFromQueue(ALint buffer_handle)
    {
        // Copy audio data from queue to playback buffers
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

            //QFile file("audio-FillBufferFromQueue.raw");
            //file.open(QIODevice::OpenModeFlag::Append);
            //file.write((char*)data_queue_[i], size);
            //file.close();

            memcpy(local_copy + offset, data_queue_[i], size);
            delete [] data_queue_[i]; // FREE MEMORY
            offset += size;
        }
        data_queue_.clear();
        data_queue_packet_sizes_.clear();

        // clear previously playback buffer 
        if (playback_buffers_[buffer_handle])
            delete [] playback_buffers_[buffer_handle]; // FREE MEMORY

        playback_buffers_[buffer_handle] = local_copy;

        
        //QFile file("audio-FillBufferFromQueue-2.raw");
        //file.open(QIODevice::OpenModeFlag::Append);
        //file.write((char*)playback_buffers_[buffer_handle], total_queue_size);
        //file.close();

        alGetError();
        alBufferData(buffer_handle, format_, playback_buffers_[buffer_handle], total_queue_size, frequency_);
        ALenum error = alGetError();
        if (error == AL_OUT_OF_MEMORY)
        {
            OpenALAudioModule::LogError("Cannot fill audio buffer: OpenAl out of memory");
            return 0;
        }
        if (error != AL_NONE)
        {
            OpenALAudioModule::LogError("Cannot fill audio buffer: Reason unknown");
            return 0;
        }
        return buffer_handle;
    }

    void SoundStream::AddData(u8 *data, uint size)
    {
        if (!add_data_mutex_.tryLock())
            return;
        int max_buffer_length_ms = 200;

        test_ ++;

        ALint empty_buffer_count = 0;
        alGetSourcei(source_, AL_BUFFERS_PROCESSED, &empty_buffer_count);
        if (empty_buffer_count <= 0 && GetReceivedAudioDataLengthMs() > max_buffer_length_ms)
        {
            OpenALAudioModule::LogDebug("Drop audio packet, no buffers for playback.");
            // All the buffers are full, we'll ignore this audio sample packet
            test_ --;
            add_data_mutex_.unlock();
            return;
        }

        StoreToQueue(data, size);

        if (empty_buffer_count <= 0 || GetReceivedAudioDataLengthMs() < max_buffer_length_ms / 2)
        {
            // we do not want to fill OpenAL buffer even if we have one available but we have stored the data to queue
            test_ --;
            add_data_mutex_.unlock();
            return;
        }

        // Now we have a ampty OpenAl buffer to fill and at enough data on queue
        ALuint buffer_handle;
        alGetError();
        alSourceUnqueueBuffers(source_, 1, &buffer_handle);
        if (alGetError() != AL_NONE)
        {
            OpenALAudioModule::LogDebug("Could not pull empty buffer from source!");
            add_data_mutex_.unlock();
            test_ --;
            return;
        }

        if (!FillBufferFromQueue(buffer_handle))
        {
            OpenALAudioModule::LogDebug("Could not fill OpenAL buffer.");
            add_data_mutex_.unlock();
            test_ --;
            return;
        }

        // Queue buffer back to sources playlist
        alSourceQueueBuffers(source_, 1, &buffer_handle);

        assert (test_ == 1);
        if (!IsPlaying())
            Play();

        add_data_mutex_.unlock();
        test_ --;
    }

    bool SoundStream::IsPlaying()
    {
        ALint state;
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
