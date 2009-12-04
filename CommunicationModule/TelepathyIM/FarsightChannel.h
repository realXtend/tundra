#ifndef incl_Communication_TelepathyIM_FarsightChannel_h
#define incl_Communication_TelepathyIM_FarsightChannel_h

#include <QObject>
#include <QMetaType>
#include <QString>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/StreamedMediaChannel>
#include <telepathy-farsight/channel.h>

namespace TelepathyIM
{
    /**
     *  // todo: make audio buffer available for AudioModule to playback
     *
     */
    class FarsightChannel : public QObject
    {
    public:
        enum Status {
        StatusDisconnected = 0,
        StatusConnecting = 1,
        StatusConnected = 2
        };    

        FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, const QString &audioSrc, const QString &audioSink, const QString &videoSrc);
      //  VideoWidget *videoPreview() const;
      //  VideoWidget *videoWidget() const;
        
        FarsightChannel::Status GetStatus() const;

        //! @param value 0 to 1
        void SetAudioPlaybackVolume(const double value);

        //! @param value 0 to 1
        void SetAudioRecordVolume(const double value);

    };

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_FarsightChannel_h