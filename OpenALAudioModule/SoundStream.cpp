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
          buffer_counter_(0)
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
            playback_buffers_[buffer_handle] = new QByteArray();
            alBufferData(buffer_handle, format_, playback_buffers_[buffer_handle]->data(), playback_buffers_[buffer_handle]->size(), frequency_);
            alSourceQueueBuffers(source_, 1, &buffer_handle); // buffer is empy at the moment, but we don't start playback yet
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
            delete playback_buffers_[i];
        }
    }

    void SoundStream::Play()
    {
        alSourcePlay(source_);
        OpenALAudioModule::LogDebug(">> Stream playback started");
    }

    int SoundStream::GetReceivedAudioDataLengthMs()
    {
        return received_audio_data_.size()*1000*8/sample_width_/frequency_;
    }

    void SoundStream::AddData(u8 *data, uint size)
    {
        add_data_mutex_.lock();
        int max_buffer_length_ms = 1500;

        ALint empty_buffer_count = 0;
        alGetSourcei(source_, AL_BUFFERS_PROCESSED, &empty_buffer_count);
        if (empty_buffer_count == 0 && GetReceivedAudioDataLengthMs() > max_buffer_length_ms)
        {
            OpenALAudioModule::LogDebug("Drop audio packet, no buffers for playback.");
            add_data_mutex_.unlock();
            return;
        }
        received_audio_data_.append((char*)data, size);

        if (empty_buffer_count == 0)
        {
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
        QByteArray* bytes = playback_buffers_[buffer_handle];
        if (!bytes)
            assert(false); // These have been created on constructor
        bytes->clear(); // Free previously reserved memory

        // copy data
        int copy_size = received_audio_data_.size();
        //int max_copy_size = 1000;
        //if (copy_size > max_copy_size)
        //    copy_size = max_copy_size;
        bytes->append(received_audio_data_, copy_size);
        received_audio_data_.clear();

        alBufferData(buffer_handle, format_, bytes->constData(), bytes->size(), frequency_);

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
