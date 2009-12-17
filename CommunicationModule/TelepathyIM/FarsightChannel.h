#ifndef incl_Communication_TelepathyIM_FarsightChannel_h
#define incl_Communication_TelepathyIM_FarsightChannel_h

#include <QObject>
#include <QMetaType>
#include <QString>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/StreamedMediaChannel>
#include <telepathy-farsight/channel.h>
#include <Foundation.h>
#include "VideoWidget.h"

namespace TelepathyIM
{
    /**
     *  Wrapper class for telepathy-farsight channel
     *
     *  // todo: - make audio buffer available for AudioModule to playback
     *           - add checks for object creation
     *  
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
            StatusConnected = 2 };    

        FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                        const QString &audioSrc, 
                        const QString &audioSink, 
                        const QString &videoSrc = "");
        ~FarsightChannel();

        FarsightChannel::Status GetStatus() const;

        //! @param value 0 to 1
        void SetAudioPlaybackVolume(const double value);

        //! @param value 0 to 1
        void SetAudioRecordVolume(const double value);

        VideoWidget* GetLocallyCapturedVideoWidget() { return locally_captured_video_widget_; };
        VideoWidget* GetReceivedVideoWidget() { return received_video_widget_; };

        int audio_stream_in_clock_rate_; // todo getter

    Q_SIGNALS:
        void statusChanged(TelepathyIM::FarsightChannel::Status status);
        void AudioStreamReceived();
        void VideoStreamReceived();

        //! When audio buffer is ready for playback
        void AudioPlaybackBufferReady(Core::u8* buffer, int buffer_size);

    public:
        GstPad *audio_in_src_pad_; // todo setter
        GstPad *video_in_src_pad_; // todo setter
        Status status_; // todo setter

    private:

        GstElement* setUpElement(const QString &element_name);

        void ConnectTfChannelEvents();
        void CreatePipeline();
        void CreateTfChannel();
        void CreateAudioInputElement(const QString &name);
        void CreateAudioPlaybackElement(const QString &audio_sink_name);
        void CreateVideoInputElement(const QString &video_src_name);
        void CreateVideoOutputElements();

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

        static void OnFakeSinkHandoff(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data);

        Tp::StreamedMediaChannelPtr tp_channel_;
        
        //GValue volume_;
        GValue input_volume_;
        TfChannel *tf_channel_; // telepathy-farsight channel, that this class basically wraps
        
        GstElement *pipeline_;
        GstBus *bus_;

        // adjustable on init
        GstElement *audio_input_;
        GstElement *video_input_;
        GstElement *video_tee_;
        GstElement *fake_audio_output_;
        
        // audio modification elements
        GstElement *audio_resample_;
        GstElement *audio_volume_;
        GstElement *audio_capsfilter_;
        GstElement *audio_convert_;

        // bins
        GstElement *audio_playback_bin_;
        GstElement *video_input_bin_;
        
        GstPad  *audio_playback_bin_sink_pad_;
        
        VideoWidget *locally_captured_video_widget_;
        VideoWidget *received_video_widget_;

        GstElement *locally_captured_video_playback_element_;
        GstElement *received_video_playback_element_;
    };

} // end of namespace: TelepathyIM

Q_DECLARE_METATYPE(TelepathyIM::FarsightChannel::Status)

#endif // incl_Communication_TelepathyIM_FarsightChannel_h
