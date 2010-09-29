// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WavLoader.h"
#include "OpenALAudioModule.h"
 
namespace OpenALAudio
{
    void ReadBytes(u8* dest, u8* src, uint& index, uint size)
    {
        memcpy(dest, &src[index], size);
        index += size;
    }
    
    u8 ReadU8(u8* src, uint& index)
    {
        u8 ret = src[index];
        index += sizeof(u8);
        return ret;
    }

    u16 ReadU16(u8* src, uint& index)
    {
        u16 ret = *((u16*)(&src[index]));
        index += sizeof(u16);
        return ret;
    }
    
    u32 ReadU32(u8* src, uint& index)
    {
        u32 ret = *((u32*)(&src[index]));
        index += sizeof(u32);
        return ret;
    }    

    bool WavLoader::LoadFromFile(Sound* sound, const std::string& filename)
    {
        boost::filesystem::path file_path(filename);
        std::ifstream file(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            OpenALAudioModule::LogError("Could not open file: " + filename + ".");
            return false;
        }

        std::vector<u8> buffer;
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        buffer.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&buffer[0], size);
        file.close();
            
        return LoadFromBuffer(sound, &buffer[0], size);
    }
     
    bool WavLoader::LoadFromBuffer(Sound* sound, u8* data, uint size)
    {        
        uint index = 0;
        
        if (index >= size) return false;
        u8 riff_text[4];
        ReadBytes(riff_text, data, index, 4);
        if (memcmp(riff_text, "RIFF", 4))
        {
            OpenALAudioModule::LogError("No RIFF chunk in WAV data");
            return false;
        }
        if (index >= size) return false;
        uint total_size = ReadU32(data, index);

        u8 wave_text[4];        
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
            u8 chunk_text[4]; 
            ReadBytes(chunk_text, data, index, 4);
            uint chunk_size = ReadU32(data, index);
            if (!memcmp(chunk_text, "fmt ", 4))
                break;
            if (!chunk_size) return false;
            index += chunk_size;
        }
        
        if (index >= size) return false;
        u16 format = ReadU16(data, index);
        u16 channels = ReadU16(data, index);
        uint frequency = ReadU32(data, index);
        uint avgbytes = ReadU32(data, index);
        uint blockalign = ReadU16(data, index);
        u16 bits = ReadU16(data, index);
        
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
        uint data_length = 0;
        for (;;)
        {
            if (index >= size)
            {
                OpenALAudioModule::LogError("No data chunk in WAV data");
                return false;
            }
            u8 chunk_text[4]; 
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
        
        std::ostringstream msg;
        msg << "Loaded WAV sound with " << channels << " channels " << bits << " bits, frequency " << frequency << " datasize " << data_length; 
        OpenALAudioModule::LogDebug(msg.str());
        
        ISoundService::SoundBuffer wav_buffer;
        wav_buffer.data_.resize(data_length);
        memcpy(&wav_buffer.data_[0], &data[index], data_length);
        wav_buffer.frequency_ = frequency;
        wav_buffer.sixteenbit_  = (bits == 16);
        wav_buffer.stereo_ = (channels == 2);
        
        return sound->LoadFromBuffer(wav_buffer);
    }
}