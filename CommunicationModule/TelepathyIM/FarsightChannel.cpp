#include "StableHeaders.h"

#include "FarsightChannel.h"
#include <TelepathyQt4/Farsight/Channel>
#include <CommunicationService.h>

namespace TelepathyIM
{
    FarsightChannel::FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                                     const QString &audio_src_name, 
                                     const QString &audio_sink_name, 
                                     const QString &video_src_name) 
                                     : QObject(0),
                                     channel_(0), tf_channel_(0), bus_(0), pipeline_(0), audio_input_(0), 
                                     audio_output_(0), video_input_(0), video_tee_(0), 
                                     audio_volume_(0),
                                     audio_resample_(0),
                                     audio_out_linked_(false),
                                     video_out_linked_(false)
    {
        try
        {
            tf_channel_ = createFarsightChannel(channel);        
        }
        catch(...)
        {
            throw Core::Exception("Cannot create TfChannel object!");
        }
        if (!tf_channel_)
        {
            LogError("Unable to construct TfChannel");
            return;
        }

        ConnectTfChannelEvents();
        
        //g_value_init (&volume_, G_TYPE_DOUBLE);
        pipeline_ = gst_pipeline_new(NULL);
        if (pipeline_ == 0)
            throw Core::Exception("Cannot create GStreamer pipeline.");
        bus_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
        if (bus_ == 0)
            throw Core::Exception("Cannot create GStreamer bus.");

        audio_input_ = setUpElement(audio_src_name);
        if (audio_input_ == 0)
            throw Core::Exception("Cannot create GStreamer audio input element.");

        audio_output_ = setUpElement(audio_sink_name);
        if (audio_output_ == 0)
            throw Core::Exception("Cannot create GStreamer audio output element.");

        fake_audio_output_ = setUpElement("fakesink");
        if (fake_audio_output_ == 0)
            throw Core::Exception("Cannot create GStreamer fake audio output element.");
        else
        {
            g_signal_connect(fake_audio_output_, "handoff", G_CALLBACK(&FarsightChannel::OnFakeSinkHandoff), this);
            g_object_set(G_OBJECT(fake_audio_output_), "signal-handoffs", (gboolean)true, this);
        }
        
        // audio modifications
        audio_resample_ = gst_element_factory_make("audioresample", NULL);
        if (audio_resample_ == 0)
            throw Core::Exception("Cannot create GStreamer audio resample element.");

        //audio_volume_  = gst_element_factory_make("volume", NULL);

        audio_playback_bin_ = gst_bin_new("audio-output-bin");
        if (audio_playback_bin_ == 0)
            throw Core::Exception("Cannot create GStreamer bin for audio playback.");

        gst_bin_add_many(GST_BIN(audio_playback_bin_), audio_resample_, audio_output_, NULL);
        gst_element_link_many(audio_resample_, audio_output_, NULL);
        // todo: Check for errors

        // add ghost pad to audio_bin_
        GstPad *sink = gst_element_get_static_pad(audio_resample_, "sink");
        audio_playback_bin_sink_ = gst_ghost_pad_new("sink", sink);
        gst_element_add_pad(GST_ELEMENT(audio_playback_bin_), audio_playback_bin_sink_);
        gst_object_unref(G_OBJECT(sink));
        gst_object_ref(audio_playback_bin_);
        gst_object_sink(audio_playback_bin_);

        GstElement *video_src;
        
        if( video_src_name.length() != 0)
        {
            // create video elems,
            video_input_bin_ = gst_bin_new("video-input-bin");
            
            GstElement *scale = gst_element_factory_make("videoscale", NULL);
            GstElement *rate = gst_element_factory_make("videorate", NULL);
            GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);
            GstElement *capsfilter = gst_element_factory_make("capsfilter", NULL);
            GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
                    "width", G_TYPE_INT, 320,
                    "height", G_TYPE_INT, 240,
                    NULL);
            g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);

            video_src = setUpElement(video_src_name);
            gst_bin_add_many(GST_BIN(video_input_bin_), video_src, scale, rate, colorspace, capsfilter, NULL);
            gst_element_link_many(video_src, scale, rate, colorspace, capsfilter, NULL);
            GstPad *src = gst_element_get_static_pad(capsfilter, "src");
            GstPad *ghost = gst_ghost_pad_new("src", src);
            Q_ASSERT(gst_element_add_pad(GST_ELEMENT(video_input_bin_), ghost));
            gst_object_unref(G_OBJECT(src));
            gst_object_ref(video_input_bin_);
            gst_object_sink(video_input_bin_);

            video_tee_ = setUpElement("tee");
            gst_object_ref(video_tee_);
            gst_object_sink(video_tee_);

            video_preview_widget_ = new VideoWidget(bus_);
            video_preview_element_ = video_preview_widget_->GetVideoSink();
            
            gst_bin_add_many(GST_BIN(pipeline_), video_input_bin_, video_tee_, video_preview_element_, NULL);
            gst_element_link_many(video_input_bin_, video_tee_, video_preview_element_, NULL);
            
            video_remote_output_widget_ = new VideoWidget(bus_);
            video_remote_output_element_ = video_remote_output_widget_->GetVideoSink();

            //video_preview_widget_->show();
            //video_remote_output_widget_->show();
        }
        // can empty pipeline be put to playing when video is not used?, lets try anyway
        gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        status_ = StatusConnecting;
        emit statusChanged(status_);
    }

    void FarsightChannel::OnFakeSinkHandoff(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data)
    {
        Foundation::Framework* framework = ((Communication::CommunicationService*)(Communication::CommunicationService::GetInstance()))->GetFramework();
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;                

        unsigned long long offset = buffer->offset;
        if (offset == 0xFFFFFFFFFFFFFFFF) // offset not set
            offset = 0;
//        static Core::u8 audio_buffer[10000];
        //for (int i=offset; i < buffer->size; i++)
        //    audio_buffer[i-offset] = buffer->data[i];
//        Core::sound_id_t sound_id = soundsystem->PlayAudioData(audio_buffer, buffer->size-offset, 8000, 16, false);
        // todo: Get actual adio caps and pass them to soundS
        Core::sound_id_t sound_id = soundsystem->PlayAudioData(buffer->data + offset, buffer->size - offset, 8000, 16, false);
        if (sound_id == 0)
            LogError("Cannot playback audio data.");

        //OpenALAudio::Sound* sound = new OpenALAudio::Sound("");
        //sound->LoadFromBuffer(audio_buffer, buffer->size-offset, 8000, true, false);
        //FarsightChannel* self = (FarsightChannel*)user_data;
        //self->sound_channel_.Play(OpenALAudio::SoundPtr(sound));
        //const Core::Vector3df listener_pos(0,0,0);
        //self->sound_channel_.Update(listener_pos);
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
        const gchar* cStrElemName = tempElemName.c_str();
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
        // todo: Check if source pad is already linked!
  //      codec->channels;
        guint media_type;

        g_object_get(stream, "media-type", &media_type, NULL);

        GstPad *pad;
        GstElement *element = 0;

        bool sink_already_linked = false;

        switch (media_type)
        {
        case TP_MEDIA_STREAM_TYPE_AUDIO:
            // element = self->audio_playback_bin_;
            element = self->fake_audio_output_; // fake audio sink
            g_object_ref(element);
            if (self->audio_out_linked_)
                sink_already_linked = true;
            break;
        case TP_MEDIA_STREAM_TYPE_VIDEO:
            element = self->video_remote_output_element_;
            if (self->video_out_linked_)
                sink_already_linked = true;
            break;
        default:
            Q_ASSERT(false);
        }

        if (sink_already_linked)
        {

        }
        else
        {
            gst_bin_add(GST_BIN(self->pipeline_), element);
        }
        pad = gst_element_get_static_pad(element, "sink");
        if (gst_pad_is_linked(pad))
            gst_pad_unlink(src, pad);
        gst_element_set_state(element, GST_STATE_PLAYING);
        
        self->status_ = StatusConnected;
        switch (media_type)
        {
        case TP_MEDIA_STREAM_TYPE_AUDIO:
            self->audio_out_linked_ = true;
            break;
        case TP_MEDIA_STREAM_TYPE_VIDEO:
            self->video_out_linked_ = true;
            break;
        default:
            Q_ASSERT(false);
        }

        emit self->statusChanged(self->status_);
    }

    gboolean FarsightChannel::onRequestResource(TfStream *stream, guint direction, gpointer data)
    {
        LogInfo("CommunicationModule: resource request");
        return TRUE;
    }

} // end of namespace: TelepathyIM
