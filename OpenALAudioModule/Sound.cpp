// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Sound.h"
#include "OpenALAudioModule.h"

namespace OpenALAudio
{
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
    
    bool Sound::LoadWavFromBuffer(Core::u8* data, Core::uint size)
    {
        DeleteBuffer();
         
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
        
        ALenum openal_format;
        if (channels == 1)
        {
            if (bits == 8)
                openal_format = AL_FORMAT_MONO8;
            else
                openal_format = AL_FORMAT_MONO16;
        }
        else
        {
            if (bits == 8)
                openal_format = AL_FORMAT_STEREO8;
            else
                openal_format = AL_FORMAT_STEREO16;
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
        
        alBufferData(handle_, openal_format, &data[index], data_length, frequency);
        size_ = data_length; 
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