#ifndef incl_Communication_TelepathyIM_FarsightChannel_h
#define incl_Communication_TelepathyIM_FarsightChannel_h

#include <QObject>
#include <QMetaType>
#include <QString>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/StreamedMediaChannel>
#include <telepathy-farsight/channel.h>
#include <Foundation.h>


namespace TelepathyIM
{
    /**
     *  // todo: make audio buffer available for AudioModule to playback
     *  // Wrapper class for telepathy-farsight channel
     */
    class FarsightChannel : public QObject
    {
        Q_OBJECT
        Q_ENUMS(Status)

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

    public:
        enum Status {
        StatusDisconnected = 0,
        StatusConnecting = 1,
        StatusConnected = 2
        };    

        FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                        const QString &audioSrc, 
                        const QString &audioSink, 
                        const QString &videoSrc = NULL);
        ~FarsightChannel();

      //  VideoWidget *videoPreview() const;
      //  VideoWidget *videoWidget() const;
        
        FarsightChannel::Status GetStatus() const;

        //! @param value 0 to 1
        void SetAudioPlaybackVolume(const double value);

        //! @param value 0 to 1
        void SetAudioRecordVolume(const double value);

    Q_SIGNALS:
        void statusChanged(FarsightChannel::Status status);

    private:

        GstElement* setUpElement(QString elemName);
        void ConnectTfChannelEvents();

        // G_CALLBACK's need static methods
        static gboolean busWatch(GstBus *bus,
                GstMessage *message, FarsightChannel *self);
        static void onClosed(TfChannel *tfChannel,
                FarsightChannel *self);
        static void onSessionCreated(TfChannel *tfChannel,
                FsConference *conference, FsParticipant *participant,
                FarsightChannel *self);
        static void onStreamCreated(TfChannel *tfChannel,
                TfStream *stream, FarsightChannel *self);
        static void onSrcPadAdded(TfStream *stream,
                GstPad *src, FsCodec *codec, FarsightChannel *self);
        static gboolean onRequestResource(TfStream *stream,
                guint direction, gpointer data);

        Tp::StreamedMediaChannelPtr channel_;
        Status status_;
        GValue volume_;
        GValue input_volume_;
        TfChannel *tf_channel_; // telepathy-farsight channel, that this class basically wraps
        
        GstElement *pipeline_;
        GstBus *bus_;

        // adjustable on init
        GstElement *audio_input_;
        GstElement *audio_output_;
        GstElement *video_input_;
        
        // audio modification elements
        GstElement *audio_resample_;
        GstElement *audio_volume_;

        GstElement *video_tee_;

        // bins
        GstElement *audio_bin_;
        GstElement *video_bin_;
        
        GstPad  *_ghost;
        //VideoWidget *videoPreview;
        //VideoWidget *videoOutput;
    };

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_FarsightChannel_h