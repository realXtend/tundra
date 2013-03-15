// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "WavLoader.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

#include <iostream>
#include <sstream>

using namespace std;

static void ReadBytes(u8* dest, const u8* src, uint& index, unsigned int numBytes)
{
    memcpy(dest, &src[index], numBytes);
    index += numBytes;
}

/// @note The following function was dead code. Bring back if needed.
/*static u8 ReadU8(const u8* src, uint& index)
{
    u8 ret = src[index];
    index += sizeof(u8);
    return ret;
}*/

static u16 ReadU16(const u8* src, uint& index)
{
    u16 ret = *((u16*)(&src[index]));
    index += sizeof(u16);
    return ret;
}

static u32 ReadU32(const u8* src, uint& index)
{
    u32 ret = *((u32*)(&src[index]));
    index += sizeof(u32);
    return ret;
}    

namespace WavLoader
{

bool IdentifyWavFileInMemory(const u8 *fileData, size_t numBytes)
{
    if (!fileData || numBytes < 4)
        return false;
    if (!memcmp(fileData, "RIFF", 4))
        return true;
    else
        return false;
}

bool LoadWavFromFileInMemory(const u8 *fileData, size_t numBytes, std::vector<u8> &dst, bool *isStereo, bool *is16Bit, int *frequency)
{
    if (!fileData || numBytes == 0)
    {
        LogError("Null input data passed in");
        return false;
    }

    if (!isStereo || !is16Bit || !frequency)
    {
        LogError("Outputs not set");
        return false;
    }

    unsigned int index = 0;
    
    u8 riff_text[4];
    ReadBytes(riff_text, fileData, index, 4);
    if (!!memcmp(riff_text, "RIFF", 4))
    {
        LogError("No RIFF chunk in WAV data");
        return false;
    }
    if (index >= numBytes) 
        return false;

    ReadU32(fileData, index);
    u8 wave_text[4];
    ReadBytes(wave_text, fileData, index, 4);
    if (!!memcmp(wave_text, "WAVE", 4))
    {
        LogError("No WAVE chunk in WAV data");
        return false;
    }
    
    // Search for the fmt chunk
    for(;;)
    {
        if (index >= numBytes)
        {
            LogError("No fmt chunk in WAV data");
            return false;
        }
        u8 chunk_text[4]; 
        ReadBytes(chunk_text, fileData, index, 4);
        unsigned int chunk_size = ReadU32(fileData, index);
        if (!memcmp(chunk_text, "fmt ", 4))
            break;
        if (!chunk_size)
            return false;
        index += chunk_size;
    }
    
    if (index >= numBytes) 
        return false;

    u16 format = ReadU16(fileData, index);
    u16 channels = ReadU16(fileData, index);
    unsigned int sampleFrequency = ReadU32(fileData, index);
    /*unsigned int avgbytes =*/ ReadU32(fileData, index);
    /*unsigned int blockalign =*/ ReadU16(fileData, index);
    u16 bits = ReadU16(fileData, index);

    if (format != 1)
    {
        LogError("Sound is not PCM data");
        return false;
    }
    if (channels != 1 && channels != 2)
    {
        LogError("Sound is not either mono or stereo");
        return false;
    }
    if (bits != 8 && bits != 16)
    {
        LogError("Sound is not either 8bit or 16bit");
        return false;
    }
                            
    // Search for the data chunk
    unsigned int data_length = 0;
    for(;;)
    {
        if (index >= numBytes)
        {
            LogError("No data chunk in WAV data");
            return false;
        }
        u8 chunk_text[4]; 
        ReadBytes(chunk_text, fileData, index, 4);
        data_length = ReadU32(fileData, index);
        if (!memcmp(chunk_text, "data", 4))
            break;
        if (!data_length) return false;
        index += data_length;
    }        
    
    if (!data_length)
    {
        LogError("Zero numBytes data chunk in WAV data");
        return false;
    }
    
    std::ostringstream msg;
    msg << "Loaded WAV sound with " << channels << " channels " << bits << " bits, frequency " << sampleFrequency << " datasize " << data_length; 
    LogDebug(msg.str());
 
    dst.clear();
    dst.insert(dst.end(), &fileData[index], &fileData[index + data_length]);
    *isStereo = (channels == 2);
    *is16Bit = (bits == 16);
    *frequency = sampleFrequency;

    return true;
}

} // ~WavLoader
