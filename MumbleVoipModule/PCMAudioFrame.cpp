#include "StableHeaders.h"
#include "PCMAudioFrame.h"

namespace MumbleVoip
{
    PCMAudioFrame::PCMAudioFrame(int sample_rate, int sample_width, int channels, char* data, int data_size):
            channels_(channels),
            sample_rate_(sample_rate),
            sample_width_(sample_width),
            data_(data),
            data_size_(data_size)
    {
        data_ = new char[data_size];
        memcpy(data_, data, data_size);
    }

    PCMAudioFrame::PCMAudioFrame(int sample_rate, int sample_width, int channels, int data_size):
            channels_(channels),
            sample_rate_(sample_rate),
            sample_width_(sample_width),
            data_size_(data_size)
    {
        data_ = new char[data_size];
    }

    PCMAudioFrame::~PCMAudioFrame()
    {
        SAFE_DELETE_ARRAY(data_);
    }
        
    char* PCMAudioFrame::DataPtr()
    {
        return data_;
    }

    int PCMAudioFrame::Channels()
    {
        return channels_;
    }
        
    int PCMAudioFrame::SampleRate()
    {
        return sample_rate_;
    }

    int PCMAudioFrame::SampleWidth()
    {
        return sample_width_;
    }
        
    int PCMAudioFrame::Samples()
    {
        return data_size_ / sample_width_;
    }

    int PCMAudioFrame::GetLengthMs()
    {
        return 1000 * Samples() / sample_rate_;
    }

    int PCMAudioFrame::GetLengthBytes()
    {
        return data_size_;
    }

} // namespace MumbleVoip
