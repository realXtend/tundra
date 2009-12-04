#include "FarsightChannel.h"

namespace TelepathyIM
{
    FarsightChannel::FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, const QString &audioSrc, const QString &audioSink, const QString &videoSrc)
    {
        // todo: Implement
    }

    FarsightChannel::Status FarsightChannel::GetStatus() const
    {
        // todo: Implement
        return StatusDisconnected;
    }
    
    void FarsightChannel::SetAudioPlaybackVolume(const double value)
    {
        // todo: Implement
    }
    
    void FarsightChannel::SetAudioRecordVolume(const double value)
    {
        // todo: Implement
    }

} // end of namespace: TelepathyIM
