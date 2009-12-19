// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_SoundStream_h
#define incl_OpenALAudio_SoundStream_h

#include <AL/al.h>
#include <AL/alc.h>
#include <QMap>
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
        void StoreToQueue(u8* data, int size);

        //! /return handle to given buffer if success. Return 0 if fails
        ALint FillBufferFromQueue(ALint);


        ALuint buffers_[MAX_BUFFER_COUNT];
        ALuint source_;
        ALenum format_;

        int current_buffer_;
        std::string name_;
        uint frequency_;
        int sample_width_;
        bool stereo_;

        std::vector<u8*> data_queue_;
        std::vector<u32> data_queue_packet_sizes_;

        // memory for OpenAL buffer objects
        QMap<ALuint, u8*> playback_buffers_;
        QMutex add_data_mutex_;
    };
}

#endif // incl_OpenALAudio_SoundStream_h
