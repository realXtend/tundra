// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Sound.h"
#include "OpenALAudioModule.h"

#include <vorbis/vorbisfile.h>


namespace OpenALAudio
{
    static const int MAX_DECODE_SIZE = 16384;
    
    class OggMemDataSource
    {
    public:
        OggMemDataSource(Core::u8* data, Core::uint size) :
            data_(data),
            size_(size),
            position_(0)    
        {
        }
            
        size_t Read(void* ptr, size_t size)
        {
            Core::uint max_read = size_ - position_;
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
            position_ = (Core::uint)new_pos;
            return 0;
        }
        
        long Tell() const
        {
            return (long)position_;
        }            
            
    private:
        Core::u8* data_;
        Core::uint size_;
        Core::uint position_;        
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
        
    void ReadBytes(Core::u8* dest, Core::u8* src, Core::uint& index, Core::uint size)
    {
        memcpy(dest, &src[index], size);
        index += size;
    }
    
    Core::u8 ReadU8(Core::u8* src, Core::uint& index)
    {
        Core::u8 ret = src[index];
        index += sizeof(Core::u8);
        return ret;
    }

    Core::u16 ReadU16(Core::u8* src, Core::uint& index)
    {
        Core::u16 ret = *((Core::u16*)(&src[index]));
        index += sizeof(Core::u16);
        return ret;
    }
    
    Core::u32 ReadU32(Core::u8* src, Core::uint& index)
    {
        Core::u32 ret = *((Core::u32*)(&src[index]));
        index += sizeof(Core::u32);
        return ret;
    }    
    
    Sound::Sound() : handle_(0), size_(0), age_(0.0)
    {
    }
    
    Sound::~Sound()
    {
        DeleteBuffer();
    }
    
    bool Sound::LoadWavFromFile(const std::string& filename)
    {
        boost::filesystem::path file_path(filename);      
        std::ifstream file(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            OpenALAudioModule::LogError("Could not open file: " + filename + ".");
            return false;
        }

        std::vector<Core::u8> buffer;
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        buffer.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&buffer[0], size);
        file.close();
            
        return LoadWavFromBuffer(&buffer[0], size);
    }
    
    bool Sound::LoadOggFromFile(const std::string& filename)
    {
        boost::filesystem::path file_path(filename);      
        std::ifstream file(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            OpenALAudioModule::LogError("Could not open file: " + filename + ".");
            return false;
        }

        std::vector<Core::u8> buffer;
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        buffer.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&buffer[0], size);
        file.close();
            
        return LoadOggFromBuffer(&buffer[0], size);    
    }
    
    bool Sound::LoadOggFromBuffer(Core::u8* data, Core::uint size)
    {
        OggVorbis_File vf;
        OggMemDataSource src(data, size);
        
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
            return false;
        }
        
        vorbis_info* vi = ov_info(&vf, -1);
        Core::uint frequency = 0;
        bool stereo = false;
        if (vi)
        {    
            std::ostringstream msg;
            msg << "Decoding ogg vorbis stream with " << vi->channels << " channels, frequency " << vi->rate; 
            OpenALAudioModule::LogDebug(msg.str()); 
            
            frequency = vi->rate;
            stereo = (vi->channels == 2); 
        }
        else          
        {
            OpenALAudioModule::LogError("No ogg vorbis stream info");
            ov_clear(&vf);
            return false;
        }

        
        std::vector<Core::u8> decoded_data;
        Core::uint decoded_bytes = 0;        
        for (;;)
        {
            decoded_data.resize(decoded_bytes + MAX_DECODE_SIZE);
            int bitstream;
            long ret = ov_read(&vf, (char*)&decoded_data[decoded_bytes], MAX_DECODE_SIZE, 0, 2, 1, &bitstream);
            if (ret <= 0)
                break;
            decoded_bytes += ret;
        }
        
        std::ostringstream msg;
        msg << "Decoded " << decoded_bytes << " of ogg vorbis sound data";
        OpenALAudioModule::LogInfo(msg.str());       
         
        ov_clear(&vf);
                           
        return LoadFromBuffer(&decoded_data[0], decoded_bytes, frequency, true, stereo);
    }
    
    bool Sound::LoadWavFromBuffer(Core::u8* data, Core::uint size)
    {        
        Core::uint index = 0;
        
        if (index >= size) return false;
        Core::u8 riff_text[4];
        ReadBytes(riff_text, data, index, 4);
        if (memcmp(riff_text, "RIFF", 4))
        {
            OpenALAudioModule::LogError("No RIFF chunk in WAV data");
            return false;
        }
        if (index >= size) return false;
        Core::uint total_size = ReadU32(data, index);

        Core::u8 wave_text[4];        
        ReadBytes(wave_text, data, index, 4);
        if (memcmp(wave_text, "WAVE", 4))
        {
            OpenALAudioModule::LogError("No WAVE chunk in WAV data");
            return false;
        }
        
        // Search for the fmt chunk
        for (;;)
        {
            if (index >= size)
            {
                OpenALAudioModule::LogError("No fmt chunk in WAV data");
                return false;
            }
            Core::u8 chunk_text[4]; 
            ReadBytes(chunk_text, data, index, 4);
            Core::uint chunk_size = ReadU32(data, index);
            if (!memcmp(chunk_text, "fmt ", 4))
                break;
            if (!chunk_size) return false;
            index += chunk_size;
        }
        
        if (index >= size) return false;
        Core::u16 format = ReadU16(data, index);
        Core::u16 channels = ReadU16(data, index);
        Core::uint frequency = ReadU32(data, index);
        Core::uint avgbytes = ReadU32(data, index);
        Core::uint blockalign = ReadU16(data, index);
        Core::u16 bits = ReadU16(data, index);
        
        if (format != 1)
        {
            OpenALAudioModule::LogError("Sound is not PCM data");
            return false;
        }
        if (channels != 1 && channels != 2)
        {
            OpenALAudioModule::LogError("Sound is not either mono or stereo");
            return false;
        }
        if (bits != 8 && bits != 16)
        {
            OpenALAudioModule::LogError("Sound is not either 8bit or 16bit");
            return false;
        }
                                
        // Search for the data chunk
        Core::uint data_length = 0;
        for (;;)
        {
            if (index >= size)
            {
                OpenALAudioModule::LogError("No data chunk in WAV data");
                return false;
            }
            Core::u8 chunk_text[4]; 
            ReadBytes(chunk_text, data, index, 4);
            data_length = ReadU32(data, index);
            if (!memcmp(chunk_text, "data", 4))
                break;
            if (!data_length) return false;
            index += data_length;
        }        
        
        if (!data_length)
        {
            OpenALAudioModule::LogError("Zero size data chunk in WAV data");
            return false;
        }
        
        if (!CreateBuffer())
            return false;
            
        std::ostringstream msg;
        msg << "Loaded WAV sound with " << channels << " channels " << bits << " bits, frequency " << frequency << " datasize " << data_length; 
        OpenALAudioModule::LogDebug(msg.str());
        
        return LoadFromBuffer(&data[index], data_length, frequency, bits == 16, channels == 2);      
    }

    bool Sound::LoadFromBuffer(Core::u8* data, Core::uint size, Core::uint frequency, bool sixteenbit, bool stereo)
    {
        DeleteBuffer();
        
        ALenum openal_format;
        if (!stereo)
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_MONO8;
            else
                openal_format = AL_FORMAT_MONO16;
        }
        else
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_STEREO8;
            else
                openal_format = AL_FORMAT_STEREO16;
        }
        
        if (!CreateBuffer())
            return false;
            
        alBufferData(handle_, openal_format, data, size, frequency);
        size_ = size; 
        return true;                
    }
            
    bool Sound::CreateBuffer()
    {    
        if (!handle_)                  
            alGenBuffers(1, &handle_);
        
        if (!handle_)
        {
            OpenALAudioModule::LogError("Could not create OpenAL sound buffer");
            return false;
        } 
        
        return true;      
    }
        
    void Sound::DeleteBuffer()
    {
        if (handle_)
        {
            alDeleteBuffers(1, &handle_);
            handle_ = 0;
            size_ = 0;
        }
    } 
}