#ifndef incl_Communication_TelepathyIM_FarsightChannel_h
#define incl_Communication_TelepathyIM_FarsightChannel_h

#include <QObject>
#include <QMetaType>
#include <QString>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/StreamedMediaChannel>
#include <telepathy-farsight/channel.h>
#include <Foundation.h>
#include <ServiceInterface.h>
#include <SoundServiceInterface.h>
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

        VideoWidget* GetPreviewVideo() { return video_preview_widget_; };
        VideoWidget* GetRemoteOutputVideo() { return video_remote_output_widget_; };
        virtual void SetAudioSourceLocation(Core::Vector3df location);

        Core::sound_id_t audio_playback_channel_;
        int audio_stream_in_clock_rate_;

    Q_SIGNALS:
        void statusChanged(TelepathyIM::FarsightChannel::Status status);

    public:
        //bool audio_out_linked_; // todo setter
        //bool video_out_linked_; // todo setter
        GstPad *audio_in_src_pad_; // todo setter
        GstPad *video_in_src_pad_; // todo setter

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

        static void OnFakeSinkHandoff(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data);

        Tp::StreamedMediaChannelPtr channel_;
        Status status_;
        //GValue volume_;
        GValue input_volume_;
        TfChannel *tf_channel_; // telepathy-farsight channel, that this class basically wraps
        
        GstElement *pipeline_;
        GstBus *bus_;

        // adjustable on init
        GstElement *audio_input_;
        GstElement *audio_output_;
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
        
        GstPad  *audio_playback_bin_sink_;
        
        VideoWidget *video_preview_widget_;
        VideoWidget *video_remote_output_widget_;

        GstElement *video_preview_element_;
        GstElement *video_remote_output_element_;


//        Core::sound_id_t audio_playback_channel_;
    };

} // end of namespace: TelepathyIM

Q_DECLARE_METATYPE(TelepathyIM::FarsightChannel::Status)

#endif // incl_Communication_TelepathyIM_FarsightChannel_h
