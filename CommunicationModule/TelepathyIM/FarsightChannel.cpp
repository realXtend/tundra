#include "StableHeaders.h"

#include "FarsightChannel.h"

#include <TelepathyQt4/Farsight/Channel>

namespace TelepathyIM
{
    FarsightChannel::FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                                     const QString &audioSrc, 
                                     const QString &audioSink, 
                                     const QString &videoSrc) 
                                     : QObject(0)
                                     // are these needed?
                                     //channel_(0), tf_channel_(0), bus_(0), pipeline_(0), audio_input_(0), 
                                     //audio_output_(0), video_input_(0), video_tee_(0), 
                                     //audio_volume_(0),
                                     //audio_resample_(0)
    {
        tf_channel_ = createFarsightChannel(channel);        
        if (!tf_channel_) {
            LogError("Unable to construct TfChannel");
            return;
        }
        ConnectTfChannelEvents();

        
        //g_value_init (&volume_, G_TYPE_DOUBLE);
        pipeline_ = gst_pipeline_new(NULL);
        bus_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));

        // create audioSrc, audioSink, videoSrc based on given vals
        audio_input_ = setUpElement(audioSrc);
        audio_output_ = setUpElement(audioSink);
        
        // audio modifications
        audio_resample_ = gst_element_factory_make("audioresample", NULL);
        //audio_volume_  = gst_element_factory_make("volume", NULL);

        audio_bin_ = gst_bin_new("audio-output-bin");
        
        gst_bin_add_many(GST_BIN(audio_bin_), audio_resample_, audio_output_, NULL);
        gst_element_link_many(audio_resample_, audio_output_, NULL);

        // add ghost pad to audio_bin_
        GstPad *sink = gst_element_get_static_pad(audio_resample_, "sink");
        audio_ghost_ = gst_ghost_pad_new("sink", sink);
        gst_element_add_pad(GST_ELEMENT(audio_bin_), audio_ghost_);
        gst_object_unref(G_OBJECT(sink));
        gst_object_ref(audio_bin_);
        gst_object_sink(audio_bin_);

        GstElement *video_src;
        
        if(videoSrc!=NULL) // If the videoSrc!=NULL we set up the pipeline with video preview elements
        {
            // create video elems,
            video_bin_ = gst_bin_new("video-input-bin");
            
            GstElement *scale = gst_element_factory_make("videoscale", NULL);
            GstElement *rate = gst_element_factory_make("videorate", NULL);
            GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);
            GstElement *capsfilter = gst_element_factory_make("capsfilter", NULL);
            GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
                    "width", G_TYPE_INT, 320,
                    "height", G_TYPE_INT, 240,
                    NULL);
            g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);

            video_src = setUpElement(videoSrc);
            gst_bin_add_many(GST_BIN(video_bin_), video_src, scale, rate, colorspace, capsfilter, NULL);
            gst_element_link_many(video_src, scale, rate, colorspace, capsfilter, NULL);
            GstPad *src = gst_element_get_static_pad(capsfilter, "src");
            GstPad *ghost = gst_ghost_pad_new("src", src);
            Q_ASSERT(gst_element_add_pad(GST_ELEMENT(video_bin_), ghost));
            gst_object_unref(G_OBJECT(src));
            gst_object_ref(video_input_);
            gst_object_sink(video_input_);

            video_tee_ = setUpElement("tee");
            //video_preview_element_ = setUpElement("glimagesink");
            video_preview_element_ = setUpElement("autovideosink");
            gst_bin_add_many(GST_BIN(pipeline_), video_bin_, video_tee_,
                    video_preview_element_, NULL);
            gst_element_link_many(video_bin_, video_tee_,
                    video_preview_element_, NULL);

            video_remote_output_element_ = setUpElement("autovideosink");
        }
        // can empty pipeline be put to playing when video is not used?, lets try anyway
        gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        status_ = StatusConnecting;
        emit statusChanged(status_);
    }

    FarsightChannel::~FarsightChannel()
    {
        if (tf_channel_) {
            g_object_unref(tf_channel_);
            tf_channel_ = 0;
        }
        if (bus_) {
            g_object_unref(bus_);
            bus_ = 0;
        }
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        if (pipeline_) {
            g_object_unref(pipeline_);
            pipeline_ = 0;
        }
        if (audio_input_) {
            g_object_unref(audio_input_);
            audio_input_ = 0;
        }
        if (audio_output_) {
            g_object_unref(audio_output_);
            audio_output_ = 0;
        }
    }

    void FarsightChannel::ConnectTfChannelEvents()
    {
        /* Set up the telepathy farsight channel */
        g_signal_connect(tf_channel_, "closed", G_CALLBACK(&FarsightChannel::onClosed), this);
        g_signal_connect(tf_channel_, "session-created", G_CALLBACK(&FarsightChannel::onSessionCreated), this);
        g_signal_connect(tf_channel_, "stream-created", G_CALLBACK(&FarsightChannel::onStreamCreated), this);
    }

    GstElement* FarsightChannel::setUpElement(QString elemName)
    {
        std::string tempElemName = elemName.toStdString();
        const char* cStrElemName = tempElemName.c_str();
        GstElement* element = gst_element_factory_make(cStrElemName, NULL);
        gst_object_ref(element);
        gst_object_sink(element);
        return element;
    }

    FarsightChannel::Status FarsightChannel::GetStatus() const
    {
        return status_;
    }
    
    void FarsightChannel::SetAudioPlaybackVolume(const double value)
    {
        if(value<0||value>1){
            LogError("Trying to set volume out of range");
            return;
        }
        double setValue = value*10; // range is from 0 to 10
        //g_value_set_double(&volume_, setValue); 
        //g_object_set_property (G_OBJECT(audio_volume_), "volume", &volume_);
    }
    
    void FarsightChannel::SetAudioRecordVolume(const double value)
    {
        if(value<0||value>1){
            LogError("Trying to set record volume out of range");
            return;
        }        
        // todo: Implement
    }

    // Link bus events to tf_channel_
    gboolean FarsightChannel::busWatch(GstBus *bus, GstMessage *message, FarsightChannel *self)
    {
        try {
            if(self->tf_channel_ == NULL) 
            {
                LogWarning("CommunicationModule: receiving bus message when tf_channel_ is NULL");
                return FALSE;
            }
            tf_channel_bus_message(self->tf_channel_, message);
            return TRUE;
        } catch(...){
            LogWarning("CommunicationModule: passing gstreamer bus message to telepathy-farsight failed");
            return FALSE;
        }
    }

    void FarsightChannel::onClosed(TfChannel *tfChannel,
        FarsightChannel *self)
    {
        self->status_ = StatusDisconnected;
        emit self->statusChanged(self->status_);
    }

    void FarsightChannel::onSessionCreated(TfChannel *tfChannel,
        FsConference *conference, FsParticipant *participant,
            FarsightChannel *self)
    {
        gst_bus_add_watch(self->bus_, (GstBusFunc) &FarsightChannel::busWatch, self);
        gst_bin_add(GST_BIN(self->pipeline_), GST_ELEMENT(conference));
        gst_element_set_state(GST_ELEMENT(conference), GST_STATE_PLAYING);
    }

    void FarsightChannel::onStreamCreated(TfChannel *tfChannel,
        TfStream *stream, FarsightChannel *self)
    {
        guint media_type;
        GstPad *sink;

        g_signal_connect(stream, "src-pad-added", G_CALLBACK(&FarsightChannel::onSrcPadAdded), self);
        g_signal_connect(stream, "request-resource", G_CALLBACK(&FarsightChannel::onRequestResource), NULL);

        g_object_get(stream, "media-type", &media_type, "sink-pad", &sink, NULL);

        GstPad *pad;

        switch (media_type) {
        case TP_MEDIA_STREAM_TYPE_AUDIO:
            gst_bin_add(GST_BIN(self->pipeline_), self->audio_input_);
            gst_element_set_state(self->audio_input_, GST_STATE_PLAYING);
            pad = gst_element_get_static_pad(self->audio_input_, "src");
            gst_pad_link(pad, sink);
            break;
        case TP_MEDIA_STREAM_TYPE_VIDEO:
            pad = gst_element_get_request_pad(self->video_tee_, "src%d");
            gst_pad_link(pad, sink);
            break;
        default:
            Q_ASSERT(false);
        }

        gst_object_unref(sink);
    }

    void FarsightChannel::onSrcPadAdded(TfStream *stream,
        GstPad *src, FsCodec *codec, FarsightChannel *self)
    {
        guint media_type;

        g_object_get(stream, "media-type", &media_type, NULL);

        GstPad *pad;
        GstElement *element = 0;

        switch (media_type) {
        case TP_MEDIA_STREAM_TYPE_AUDIO:
            element = self->audio_output_;
            g_object_ref(element);
            break;
        case TP_MEDIA_STREAM_TYPE_VIDEO:
            //element = self->video_output_->element();
            break;
        default:
            Q_ASSERT(false);
        }

        gst_bin_add(GST_BIN(self->pipeline_), element);

        pad = gst_element_get_static_pad(element, "sink");
        gst_element_set_state(element, GST_STATE_PLAYING);
        
        gst_pad_link(src, pad);

        self->status_ = StatusConnected;
        emit self->statusChanged(self->status_);
    }

    gboolean FarsightChannel::onRequestResource(TfStream *stream,
        guint direction, gpointer data)
    {
        LogInfo("CommunicationModule: resource request");
        return TRUE;
    }


} // end of namespace: TelepathyIM
