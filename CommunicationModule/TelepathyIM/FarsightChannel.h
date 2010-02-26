// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_FarsightChannel_h
#define incl_Communication_TelepathyIM_FarsightChannel_h

#include <QObject>
#include <QMetaType>
#include <QString>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/Farsight/Channel>
#include <Foundation.h>
#include "VideoWidget.h"
#include "ModuleLoggingFunctions.h"
#include "CoreThread.h"

namespace TelepathyIM
{
    /**
     *  Wrapper class for telepathy-farsight channel
     *
     *  // todo: - make audio buffer available for AudioModule to playback
     *           - add checks for object creation
     *  
     */
    class FarsightChannel : public Communication::AudioDataProducerInterface
    {
        Q_OBJECT
        Q_ENUMS(Status)

        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

        //! Max size for audio buffer for received raw audio data in bytes
        //! The whole buffer will be used as a ring buffer.
        static const int AUDIO_BUFFER_MAX_SIZE = 32768;

        //! The actual efective audio buffer size in bytes will be calculated at run time
        //! based on this value. 
        static const int AUDIO_BUFFER_SIZE_MS = 1000;

    public:

        enum Status { StatusDisconnected = 0, StatusConnecting = 1, StatusConnected = 2 };    

        FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                        const QString &audio_src_name, 
                        const QString &video_src_name = "",
                        const QString &video_sink_name = "");
        ~FarsightChannel();

        void Close();

        bool IsAudioSupported() const { return audio_supported_; } 
        bool IsVideoSupported() const { return video_supported_; }

        FarsightChannel::Status GetStatus() const;

        //! @param value 0 to 1
        //! NOT IMPLEMENTED
        void SetAudioPlaybackVolume(const double value);

        //! @param value 0 to 1
        //! NOT IMPLEMENTED
        void SetAudioRecordVolume(const double value);

        //! @return VideoPlaybackWidget representing captured video input eg. web camera
        VideoWidget* GetLocallyCapturedVideoWidget();

        //! @return VideoPlaybackWidget representing received video input eg. from another session participant web camera.
        VideoWidget* GetReceivedVideoWidget();

        //! Event handler for handoff signal of fakesink object.
        //! Stored received raw audio data to internal buffer by calling HandleAudioData method.
        static void OnFakeSinkHandoff(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data);

        //! Return true if incoming audio stream is connected return false if not.
        bool IncomingAudioStreamConnected() { return audio_in_src_pad_; }

        //! Return true if incoming video stream is connected return false if not.
        bool IncomingVideoStreamConnected() { return video_in_src_pad_; }

        //! Set the gstreamer pipeline to NULL state
        void StopPipeline();

        //! @return sample rate aka. sampling frequency. Return -1 if sample rate is unknwon.
        virtual int GetSampleRate() const;

        //! @return sample width as bits. Return -1 if sample rate is unknwon.
        virtual int GetSampleWidth() const;

        //! @return channel count. Usually returns 1 for mono or 2 for stereo stream
        virtual int GetChannelCount() const;

        //! @return count of available bytes in buffer
        virtual int GetAvailableAudioDataLength() const;

        //! Fill given buffer with audio data up to given max count of bytes or bytes available in buffer.
        //! The audio data is removed from local buffer when data is written to given buffer.
        //! @return the number of bytes wrote in given buffer
        virtual int GetAudioData(u8* buffer, int max);

        GstPad *audio_in_src_pad_; // todo setter
        GstPad *video_in_src_pad_; // todo setter
        Status status_; // todo setter
        guint bus_watch_; // todo: setter
        int audio_stream_in_clock_rate_; // todo getter

        // G_CALLBACK's need static methods
        static gboolean busWatch(GstBus *bus, GstMessage *message, FarsightChannel *self);
        static void onClosed(TfChannel *tfChannel, FarsightChannel *self);
        static void onSessionCreated(TfChannel *tfChannel,  FsConference *conference, FsParticipant *participant, FarsightChannel *self);
        static void onStreamCreated(TfChannel *tfChannel, TfStream *stream, FarsightChannel *self);
        static void onSrcPadAdded(TfStream *stream, GstPad *src, FsCodec *codec, FarsightChannel *self);
        static gboolean onRequestResource(TfStream *stream, guint direction, gpointer data);

        //! For internal use
        virtual void HandleAudioData(u8* data, int size, int rate, int widht, int channels);

    Q_SIGNALS:

        //! Triggered when status is changed
        void StatusChanged(TelepathyIM::FarsightChannel::Status status);

        //! Triggered when incoming audio stream stream become active
        void AudioStreamReceived();

        //! Triggered when incoming video stream become active
        void VideoStreamReceived();

        // (private)
        void SrcPadAdded(TfStream*, GstPad*, FsCodec*);

    private:

        GstElement* setUpElement(const QString &element_name);
        void CreatePipeline();
        void CreateTfChannel();
        void CreateAudioInputElement(const QString &name);
        void CreateAudioPlaybackElement();
        void CreateVideoInputElement(const QString &video_src_name);

        Tp::StreamedMediaChannelPtr tp_channel_;
        QString video_sink_name_;
        
        GValue input_volume_;
        TfChannel *tf_channel_; // telepathy-farsight channel, that this class basically wraps
        
        GstElement *pipeline_;
        GstBus *bus_;

        GstElement *audio_input_;
        GstElement *video_input_;
        GstElement *video_tee_;
        GstElement *fake_audio_output_;
        GstElement *audio_output_;
        
        // audio modification elements
        GstElement *audio_resample_;
        GstElement *audio_volume_;
        GstElement *audio_capsfilter_;
        GstElement *audio_convert_;

        // bins
        GstElement *audio_playback_bin_;
        GstElement *video_input_bin_;
        
        GstPad  *audio_playback_bin_sink_pad_;
        
        VideoWidget* locally_captured_video_widget_;
        VideoWidget* received_video_widget_;

        GstElement *locally_captured_video_playback_element_;
        GstElement *received_video_playback_element_;

        gulong on_closed_g_signal_;
        gulong on_session_created_g_signal_;
        gulong on_stream_created_g_signal_;

        Mutex audio_queue_mutex_;

        u8 audio_buffer_[AUDIO_BUFFER_MAX_SIZE];  // a ring buffer for received audio data
        int read_cursor_;
        int write_cursor_;
        int available_audio_data_length_;

        int received_sample_rate_;
        int received_sample_width_;
        int received_channel_count_;

        bool audio_supported_;
        bool video_supported_;

        QMutex incoming_video_widget_mutex_;

    private slots:
        void LinkIncomingSourcePad(TfStream *stream, GstPad *src_pad, FsCodec *codec);
    };

} // end of namespace: TelepathyIM

Q_DECLARE_METATYPE(TelepathyIM::FarsightChannel::Status)

#endif // incl_Communication_TelepathyIM_FarsightChannel_h
