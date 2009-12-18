// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_SoundStream_h
#define incl_OpenALAudio_SoundStream_h

#include <AL/al.h>
#include <AL/alc.h>
#include <QByteArray>
#include <QMap>
#include <QBuffer>
#include <QMutex>

// todo: Change there to static member variables
#define MAX_BUFFER_COUNT 2

namespace OpenALAudio
{
    class SoundStream
    {
    
    public:
        SoundStream(std::string stream_name, uint frequency, int sample_width, bool stereo);
        virtual ~SoundStream();

        
        void Play();
        
        //! SoundStream object will store given data to it's internal data queue so 
        //! the caller of this method can free the given data block right after this method call
        //! TODO: We would safe data copy if we just signal the caller when we have played the data
        //!       and it can free the data block.
        void AddData(u8 *data, uint size);
        bool IsPlaying();
        void SetPosition(const Vector3df& position);

    private:
        void Release();
        int GetReceivedAudioDataLengthMs();

        ALuint buffers_[MAX_BUFFER_COUNT];
        ALuint source_;
        ALenum format_;

        int current_buffer_;
        std::string name_;
        uint frequency_;
        int sample_width_;
        bool stereo_;

        int buffer_counter_;

        //! Received audio data waiting for move to playback_buffers_object
        QByteArray received_audio_data_;

        // memory for OpenAL buffer objects
        QMap<ALuint, QByteArray*> playback_buffers_;
        QMutex add_data_mutex_;
    };
}

#endif // incl_OpenALAudio_SoundStream_h
