// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_SoundStream_h
#define incl_OpenALAudio_SoundStream_h

#include <AL/al.h>
#include <AL/alc.h>

#define BUFFER_COUNT 25

namespace OpenALAudio
{
    class SoundStream
    {
    
    public:
        SoundStream(std::string stream_name, uint frequency, int sample_width, bool stereo);
        virtual ~SoundStream();

        void Release();
        void Play();
        void AddBuffer(u8 *data, uint size);
        void FillBuffer(u8 *data, uint size);
        bool IsPlaying();

        void SetPosition(Vector3df position);

        

    private:
        ALuint buffers_[BUFFER_COUNT];
        ALuint source_;
        ALenum format_;

        int current_buffer_;
        std::string name_;
        uint frequency_;
        int sample_width_;
        bool stereo_;

        int counter_;
    };
}

#endif // incl_OpenALAudio_SoundStream_h