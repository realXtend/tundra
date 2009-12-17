// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_SoundStream_h
#define incl_OpenALAudio_SoundStream_h

#include <AL/al.h>
#include <AL/alc.h>

// todo: Change there to static member variables
#define MAX_BUFFER_COUNT 30
#define DATA_QUEUE_SIZE 30

namespace OpenALAudio
{
    class SoundStream
    {
    
    public:
        SoundStream(std::string stream_name, uint frequency, int sample_width, bool stereo);
        virtual ~SoundStream();

        void Release();
        void Play();
        
        //! SoundStream object will store given data to it's internal data queue so 
        //! the caller of this method can free the given data block right after this method call
        //! TODO: We would safe data copy if we just signal the caller when we have played the data
        //!       and it can free the data block.
        void AddData(u8 *data, uint size);
        bool IsPlaying();
        void SetPosition(Vector3df position);

    private:
        //! /return handle to OpenAL buffer object with given data
        //! Does craete new OpenAL buffer object of total buffer count is lesser than BUFFER_COUNT.
        //! Anotherwise it recycles exist buffers.
        //! Used by AddData method.
        ALuint GetBufferWithData(u8* data, uint size);

        //! Store given data to internal data queue. If data queue is full then does nothing.
        //! Deletes previously reserved memory on same data queue slot before fill it with new data.
        //! Rest of data will be freed on deconstructor
        //!
        //! Only locally stored data is playwith with OpenAL buffers so caller of AddData method
        //! Can free the data immediately after method AddData call.
        //! /return pointer to stored data if there was space on data queue. Otherwise return 0.
        u8* StoreData(u8* data, u32 size);
        void ReleaseData();

        ALuint buffers_[MAX_BUFFER_COUNT];
        ALuint source_;
        ALenum format_;
        u8* data_queque_[DATA_QUEUE_SIZE];

        //! The next index in data_queue to store audio data
        int next_free_data_queue_index_; 

        //! count of used data_queue indexes
        int free_data_queque_index_count_;

        int current_buffer_;
        std::string name_;
        uint frequency_;
        int sample_width_;
        bool stereo_;

        int buffer_counter_;
    };
}

#endif // incl_OpenALAudio_SoundStream_h
