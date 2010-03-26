// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VorbisDecoder.h"
#include "OpenALAudioModule.h"
#include "Profiler.h"

#include <vorbis/vorbisfile.h>

namespace OpenALAudio
{
    static const int MAX_DECODE_SIZE = 16384;
    
    class OggMemDataSource
    {
    public:
        OggMemDataSource(u8* data, uint size) :
            data_(data),
            size_(size),
            position_(0)    
        {
        }
            
        size_t Read(void* ptr, size_t size)
        {
            uint max_read = size_ - position_;
            if (size > max_read) size = max_read;
            if (size)
            {
                memcpy(ptr, &data_[position_], size);
                position_ += size;
            }
            return size;
        }
            
        int Seek(ogg_int64_t offset, int whence)
        {
            ogg_int64_t new_pos = position_;
            switch (whence)
            {
            case SEEK_SET:
                new_pos = offset;
                break;
                
            case SEEK_CUR:
                new_pos += offset;
                break;
                
            case SEEK_END:
                new_pos = size_ + offset;
                break;
            }    
             
            if ((new_pos < 0) || (new_pos > size_))
                return -1;
            position_ = (uint)new_pos;
            return 0;
        }
        
        long Tell() const
        {
            return (long)position_;
        }            
            
    private:
        u8* data_;
        uint size_;
        uint position_;        
    };

    size_t OggReadCallback(void* ptr, size_t size, size_t nmemb, void* datasource)
    {
        OggMemDataSource* source = (OggMemDataSource*)datasource;
        return source->Read(ptr, size * nmemb);         
    }
    
    int OggSeekCallback(void* datasource, ogg_int64_t offset, int whence)
    {
        OggMemDataSource* source = (OggMemDataSource*)datasource;
        return source->Seek(offset, whence);
    }
    
    long OggTellCallback(void* datasource)
    {   
        OggMemDataSource* source = (OggMemDataSource*)datasource;
        return source->Tell();
    }

    VorbisDecoder::VorbisDecoder() :
        Foundation::ThreadTask("VorbisDecoder")
    {
    }
    
    void VorbisDecoder::Work()
    {
        while (ShouldRun())
        {
            WaitForRequests();
            
            VorbisDecodeRequestPtr request = GetNextRequest<VorbisDecodeRequest>();
            if (request)
            {
                {
                    PROFILE(VorbisDecoder_Decode);
                    PerformDecode(request);
                }
            }

            RESETPROFILER
        }
    }
    
    void VorbisDecoder::PerformDecode(VorbisDecodeRequestPtr request)
    {
        if (!request)
            return;

        VorbisDecodeResultPtr result(new VorbisDecodeResult());

        result->name_ = request->name_;
        result->frequency_ = 0;
        result->stereo_ = false;
        
        OggVorbis_File vf;
        OggMemDataSource src(&request->buffer_[0], request->buffer_.size());
        
        ov_callbacks cb;
        cb.read_func = &OggReadCallback;
        cb.seek_func = &OggSeekCallback;
        cb.tell_func = &OggTellCallback;
        cb.close_func = 0;
                       
        int ret = ov_open_callbacks(&src, &vf, 0, 0, cb);
        if (ret < 0)
        {
            OpenALAudioModule::LogError("Not ogg vorbis format");
            ov_clear(&vf);
            QueueResult<VorbisDecodeResult>(result);
            return;
        }
        
        vorbis_info* vi = ov_info(&vf, -1);
        if (vi)
        {    
            std::ostringstream msg;
            msg << "Decoding ogg vorbis stream with " << vi->channels << " channels, frequency " << vi->rate; 
            OpenALAudioModule::LogDebug(msg.str()); 
            
            result->frequency_ = vi->rate;
            result->stereo_ = (vi->channels == 2); 
        }
        else          
        {
            OpenALAudioModule::LogError("No ogg vorbis stream info");
            ov_clear(&vf);
            QueueResult<VorbisDecodeResult>(result);
            return;
        }
 
        uint decoded_bytes = 0;        
        for (;;)
        {
            result->buffer_.resize(decoded_bytes + MAX_DECODE_SIZE);
            int bitstream;
            long ret = ov_read(&vf, (char*)&result->buffer_[decoded_bytes], MAX_DECODE_SIZE, 0, 2, 1, &bitstream);
            if (ret <= 0)
                break;
            decoded_bytes += ret;
        }
        
        result->buffer_.resize(decoded_bytes);
        
        std::ostringstream msg;
        msg << "Decoded " << decoded_bytes << " bytes of ogg vorbis sound data";
        OpenALAudioModule::LogDebug(msg.str());       
         
        ov_clear(&vf);                                      
        QueueResult<VorbisDecodeResult>(result);
    }
}