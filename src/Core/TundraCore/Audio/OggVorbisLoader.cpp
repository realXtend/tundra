// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MemoryLeakCheck.h"
#include "OggVorbisLoader.h"
#include "LoggingFunctions.h"

#include <sstream>

#ifndef TUNDRA_NO_AUDIO
#include <vorbis/vorbisfile.h>
#endif

#ifndef TUNDRA_NO_AUDIO
namespace
{

class OggMemDataSource
{
public:
    OggMemDataSource(const u8* data, size_t size) :
        data_(data),
        size_(size),
        position_(0)
    {
    }

    size_t Read(void* ptr, size_t size)
    {
        size_t max_read = size_ - position_;
        if (size > max_read)
            size = max_read;
        if (size)
        {
            memcpy(ptr, &data_[position_], size);
            position_ += size;
        }
        return size;
    }

    int Seek(ogg_int64_t offset, int whence)
    {
        size_t new_pos = position_;
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

        if (new_pos < 0 || new_pos > size_)
            return -1;
        position_ = new_pos;
        return 0;
    }

    long Tell() const
    {
        return (long)position_;
    }

private:
    const u8* data_;
    size_t size_;
    size_t position_;
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

} // ~unnamed namespace
#endif

namespace OggVorbisLoader
{

bool LoadOggVorbisFromFileInMemory(const u8 *fileData, size_t numBytes, std::vector<u8> &dst, bool *isStereo, bool *is16Bit, int *frequency)
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
    
#ifndef TUNDRA_NO_AUDIO
    OggVorbis_File vf;
    OggMemDataSource src(fileData, numBytes);
    
    ov_callbacks cb;
    cb.read_func = &OggReadCallback;
    cb.seek_func = &OggSeekCallback;
    cb.tell_func = &OggTellCallback;
    cb.close_func = 0;
                   
    int ret = ov_open_callbacks(&src, &vf, 0, 0, cb);
    if (ret < 0)
    {
        LogError("Not ogg vorbis format");
        ov_clear(&vf);
        return false;
    }
    
    vorbis_info* vi = ov_info(&vf, -1);
    if (!vi)
    {
        LogError("No ogg vorbis stream info");
        ov_clear(&vf);
        return false;
    }

    std::ostringstream msg;
    msg << "Decoding ogg vorbis stream with " << vi->channels << " channels, frequency " << vi->rate; 
//    LogDebug(msg.str()); 

    *frequency = vi->rate;
    *isStereo = (vi->channels > 1);
    if (vi->channels != 1 && vi->channels != 2)
        LogWarning("Warning: Loaded Ogg Vorbis data contains an unsupported number of channels: " + QString::number(vi->channels));

    uint decoded_bytes = 0;
    dst.clear();
    for(;;)
    {
        static const int MAX_DECODE_SIZE = 16384;
        dst.resize(decoded_bytes + MAX_DECODE_SIZE);
        int bitstream;
        long ret = ov_read(&vf, (char*)&dst[decoded_bytes], MAX_DECODE_SIZE, 0, 2, 1, &bitstream);
        if (ret <= 0)
            break;
        decoded_bytes += ret;
    }
    
    dst.resize(decoded_bytes);

    {
        std::ostringstream msg;
        msg << "Decoded " << decoded_bytes << " bytes of ogg vorbis sound data";
//        LogDebug(msg.str());
    }
     
    ov_clear(&vf);
    return true;
#else
    return false;
#endif
}

} // ~OggVorbisLoader
