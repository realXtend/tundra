#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CoreException.h"
#include "FarsightChannel.h"
#include "CoreDefines.h"
#include <TelepathyQt4/Farsight/Channel>
#include <QFile>

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    FarsightChannel::FarsightChannel(const Tp::StreamedMediaChannelPtr &channel, 
                                     const QString &audio_src_name, 
                                     const QString &video_src_name,
                                     const QString &video_sink_name) :
                                     tp_channel_(channel),
                                     tf_channel_(0),
                                     bus_(0),
                                     pipeline_(0),
                                     audio_input_(0), 
                                     video_input_(0),
                                     video_tee_(0), 
                                     audio_volume_(0),
                                     audio_resample_(0),
                                     audio_in_src_pad_(0),
                                     video_in_src_pad_(0),
                                     audio_stream_in_clock_rate_(0),
                                     audio_capsfilter_(0),
                                     audio_convert_(0),
                                     audio_playback_bin_(0),
                                     locally_captured_video_widget_(0),
                                     received_video_widget_(0),
                                     on_closed_g_signal_(0),
                                     on_session_created_g_signal_(0),
                                     on_stream_created_g_signal_(0),
                                     bus_watch_(0),
                                     locally_captured_video_playback_element_(0),
                                     video_input_bin_(0),
                                     read_cursor_(0),
                                     write_cursor_(0),
                                     available_audio_data_length_(0),
                                     audio_supported_(false),
                                     video_supported_(false),
                                     video_sink_name_(video_sink_name)

    {
        connect(this, SIGNAL( SrcPadAdded(TfStream*, GstPad*, FsCodec*) ), SLOT( LinkIncomingSourcePad(TfStream*, GstPad*, FsCodec*) ), Qt::QueuedConnection );

        try
        {
            CreateTfChannel();
            CreatePipeline();
            CreateAudioInputElement(audio_src_name);
            CreateAudioPlaybackElement();
        }
        catch(Exception &e)
        {
            Close();
            throw e;
        }
        audio_supported_ = true; 

        if( video_src_name.length() != 0)
        {
            try
            {
                locally_captured_video_widget_ = new VideoWidget(bus_, 0, "captured_video", video_sink_name);
                locally_captured_video_playback_element_ = locally_captured_video_widget_->GetVideoPlaybackElement();
                LogDebug("VideoPlaybackWidget created for locally captured video stream.");

                received_video_widget_ = new VideoWidget(bus_, 0, "received_video", video_sink_name_);
                received_video_playback_element_ = received_video_widget_->GetVideoPlaybackElement();
                LogDebug("VideoPlaybackWidget created for received video stream.");

                CreateVideoInputElement(video_src_name);
                video_supported_ = true;
            }
            catch(Exception &/*e*/)
            {
                LogError("Cannot create video elements.");
            }
        }

        GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            LogError("Cannot start pipeline.");
        }

        status_ = StatusConnecting;
        emit StatusChanged(status_);
    }

    FarsightChannel::~FarsightChannel()
    {
        Close();
    }

    void FarsightChannel::Close()
    {
        StopPipeline();

        if (locally_captured_video_widget_)
        {
            locally_captured_video_widget_->close();
             SAFE_DELETE(locally_captured_video_widget_);
        }
        if (received_video_widget_)
        {
            received_video_widget_->close();
            SAFE_DELETE(received_video_widget_);
        }
        if (tf_channel_)
        {
            g_signal_handler_disconnect(tf_channel_, on_closed_g_signal_);
            g_signal_handler_disconnect(tf_channel_, on_session_created_g_signal_);
            g_signal_handler_disconnect(tf_channel_, on_stream_created_g_signal_);
            tf_channel_ = 0;
        }
        if (bus_)
        {
            if (bus_watch_)
                g_source_remove(bus_watch_);
            g_object_unref(bus_);
            bus_ = 0;
        }
        if (video_input_bin_)
        {
            g_object_unref(video_input_bin_);
            video_input_bin_ = 0;
        }
        if (audio_input_) 
        {
            audio_input_ = 0;
        }
        if (audio_playback_bin_)
        {
            audio_playback_bin_ = 0;
        }
        if (pipeline_) 
        {
            g_object_unref(pipeline_);
            pipeline_ = 0;
        }
    }

    void FarsightChannel::StopPipeline()
    {
        if (pipeline_)
            gst_element_set_state(pipeline_, GST_STATE_NULL);
    }

    void FarsightChannel::CreateTfChannel()
    {
        try
        {
            tf_channel_ = createFarsightChannel(tp_channel_);        
        }
        catch(...)
        {
            throw Exception("Cannot create TfChannel object!");
        }
        if (!tf_channel_)
        {
            LogError("Unable to construct TfChannel");
            return;
        }

        /* Set up the telepathy farsight channel */
        on_closed_g_signal_ = g_signal_connect(tf_channel_, "closed", G_CALLBACK(&FarsightChannel::onClosed), this);
        on_session_created_g_signal_ = g_signal_connect(tf_channel_, "session-created", G_CALLBACK(&FarsightChannel::onSessionCreated), this);
        on_stream_created_g_signal_ = g_signal_connect(tf_channel_, "stream-created", G_CALLBACK(&FarsightChannel::onStreamCreated), this);
    }

    void FarsightChannel::CreatePipeline()
    {
        pipeline_ = gst_pipeline_new(NULL);
        if (!pipeline_)
            throw Exception("Cannot create GStreamer pipeline.");
        bus_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
        if (!bus_)
            throw Exception("Cannot create GStreamer bus.");
    }

    void FarsightChannel::CreateAudioInputElement(const QString & name)
    {
        audio_input_ = setUpElement(name);
        if (!audio_input_)
            throw Exception("Cannot create GStreamer audio input element.");
    }

    void FarsightChannel::CreateAudioPlaybackElement()
    {
        audio_playback_bin_ = gst_bin_new("audio-output-bin");
        if (audio_playback_bin_ == 0)
            throw Exception("Cannot create GStreamer bin for audio playback.");

        fake_audio_output_ = setUpElement("fakesink");
        if (fake_audio_output_ == 0)
            throw Exception("Cannot create GStreamer fake audio output element.");
        else
        {
            g_signal_connect(fake_audio_output_, "handoff", G_CALLBACK(&FarsightChannel::OnFakeSinkHandoff), this);
            g_object_set(G_OBJECT(fake_audio_output_), "signal-handoffs", TRUE, NULL);
        }

        // audio modifications
        audio_resample_ = gst_element_factory_make("audioresample", NULL);
        if (audio_resample_ == 0)
            throw Exception("Cannot create GStreamer audio resample element.");

        audio_capsfilter_ = gst_element_factory_make("capsfilter", NULL);
        GstCaps *audio_caps = gst_caps_new_simple("audio/x-raw-int",
            "channels", G_TYPE_INT, 1,
            "width", G_TYPE_INT, 16,
//            "depth", G_TYPE_INT, 16,
            "rate", G_TYPE_INT, 16000,
            "signed", G_TYPE_BOOLEAN, true,
//            "endianess", G_TYPE_INT, 1234,
            NULL);
        g_object_set(G_OBJECT(audio_capsfilter_), "caps", audio_caps, NULL);

        //audio_convert_ = gst_element_factory_make("audioconvert", NULL);
        //if (audio_convert_ == 0)
        //    throw Exception("Cannot create GStreamer audio convert element.");

        gst_bin_add_many(GST_BIN(audio_playback_bin_), audio_resample_, fake_audio_output_, NULL);
        gboolean ok = gst_element_link_many(audio_resample_, fake_audio_output_, NULL);
        if (!ok)
        {
            QString error_message = "Cannot link elements for audio playback bin.";
            LogError(error_message.toStdString());
            throw Exception(error_message.toStdString().c_str());
        }

        // add ghost pad to audio_bin_
        GstPad *sink = gst_element_get_static_pad(audio_resample_, "sink");
        audio_playback_bin_sink_pad_ = gst_ghost_pad_new("sink", sink);
        gst_element_add_pad(GST_ELEMENT(audio_playback_bin_), audio_playback_bin_sink_pad_);
        gst_object_unref(G_OBJECT(sink));
        gst_object_ref(audio_playback_bin_);
        gst_object_sink(audio_playback_bin_);
    }

    void FarsightChannel::CreateVideoInputElement(const QString &video_src_name)
    {
        video_input_bin_ = gst_bin_new("video-input-bin");
        if (!video_input_bin_)
        {
            QString error_message("Cannot create bin for video input");
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }
        
        GstElement *scale = gst_element_factory_make("videoscale", NULL);
        if (!scale)
        {
            QString error_message = "Cannot create scale element for video input";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }

        GstElement *rate = gst_element_factory_make("videorate", NULL);
        if (!rate)
        {
            QString error_message = "Cannot create rate element for video input";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }

        GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);
        if (!colorspace)
        {
            QString error_message = "Cannot create colorspace element for video input";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }

        GstElement *capsfilter = gst_element_factory_make("capsfilter", NULL);
        if (!capsfilter)
        {
            QString error_message = "Cannot create capsfilter element for video input";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }

        GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
        //GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb",
                "width", G_TYPE_INT, 320,
                "height", G_TYPE_INT, 240,
                NULL);
        g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);

        video_input_ = gst_element_factory_make(video_src_name.toStdString().c_str(), NULL);
        if (!video_input_)
        {
            QString error_message = "Cannot create video src element for video input";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }
//        QString test = gst_element_get_name(video_input_);

        gst_bin_add_many(GST_BIN(video_input_bin_), video_input_, scale, rate, colorspace, capsfilter, NULL);
        bool ok = gst_element_link_many(video_input_, scale, rate, colorspace, capsfilter, NULL);
        if (!ok)
        {
            QString error_message = "Cannot link: video_input_ ! scale ! rate ! colorspace ! capsfilter ";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }

        GstPad *src = gst_element_get_static_pad(capsfilter, "src");
        GstPad *ghost = gst_ghost_pad_new("src", src);
        if (!ghost || !src)
        {
            QString error_message = "Cannot create ghost bad for video_input_bin_";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }
        ok = gst_element_add_pad(GST_ELEMENT(video_input_bin_), ghost);
        if (!ok)
        {
            QString error_message = "Cannot add ghost pad to video_input_bin_";
            LogError(error_message.toStdString());
            throw(Exception(error_message.toStdString().c_str()));
        }
        gst_object_unref(G_OBJECT(src));
        gst_object_ref(video_input_bin_);
        gst_object_sink(video_input_bin_);

        video_tee_ = setUpElement("tee");

        if (GST_ELEMENT(locally_captured_video_playback_element_))
        {
            gst_bin_add_many(GST_BIN(pipeline_), video_input_bin_, video_tee_, locally_captured_video_playback_element_, NULL);
            ok = gst_element_link_many(video_input_bin_, video_tee_, locally_captured_video_playback_element_, NULL);
            if (!ok)
            {
                QString error_message = "Cannot link: video_input_bin_ ! video_tee_ ! locally_captured_video_playback_element_";
                LogError(error_message.toStdString());
                gst_bin_remove_many(GST_BIN(pipeline_), video_input_bin_, video_tee_, locally_captured_video_playback_element_, NULL);
                throw(Exception(error_message.toStdString().c_str()));
            }
        }
        else
        {
            QString error_message = "locally_captured_video_playback_element_ is NULL";
            LogError(error_message.toStdString());

            gst_bin_add_many(GST_BIN(pipeline_), video_input_bin_, video_tee_, NULL);
            ok = gst_element_link_many(video_input_bin_, video_tee_, NULL);
            if (!ok)
            {
                QString error_message = "Cannot link: video_input_bin_ ! video_tee_ ";
                LogError(error_message.toStdString());
                gst_bin_remove_many(GST_BIN(pipeline_), video_input_bin_, video_tee_, NULL);
                throw(Exception(error_message.toStdString().c_str()));
            }
        }
    }

    void FarsightChannel::OnFakeSinkHandoff(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data)
    {
        FarsightChannel* self = (FarsightChannel*)user_data;

        static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
        g_static_mutex_lock (&mutex);
        gst_buffer_ref(buffer);

        int rate = 0;
        int channels = 0;
        int width = 0;
        GstCaps *caps;
        GstStructure *structure;
        caps = gst_buffer_get_caps(buffer);
        structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "rate", &rate);
        gst_structure_get_int(structure, "channels", &channels);
        gst_structure_get_int(structure, "width", &width);
        gst_caps_unref(caps);

        if (GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_PREROLL))
        {
            LogInfo("Drop fakesink buffer: Preroll audio data packet.");
            gst_buffer_unref(buffer);
            g_static_mutex_unlock (&mutex);
            return;
        }
        if (GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_GAP))
        {
            LogInfo("Drop fakesink buffer: Caps audio data packet.");
            gst_buffer_unref(buffer);
            g_static_mutex_unlock (&mutex);
            return;
        }
        if (GST_BUFFER_DURATION(buffer) == 0)
        {
            LogInfo("Drop fakesink buffer: Got audio data packet with 0 duration");
            gst_buffer_unref(buffer);
            g_static_mutex_unlock (&mutex);
            return;
        }
        if (GST_BUFFER_IS_DISCONT(buffer))
        {
            LogInfo("Drop fakesink buffer: Got disconnect audio data packet.");
            gst_buffer_unref(buffer);
            g_static_mutex_unlock (&mutex);
            return;
        }

        if (GST_BUFFER_OFFSET_IS_VALID(buffer))
        {
            guint64 offset = GST_BUFFER_OFFSET (buffer);
        }
        if (GST_BUFFER_OFFSET_END_IS_VALID(buffer))
        {
            guint64 offset = GST_BUFFER_OFFSET_END(buffer);
        }

        u8* data = GST_BUFFER_DATA(buffer);
        u32 size = GST_BUFFER_SIZE(buffer);

        self->HandleAudioData(data, size, rate, width, channels);
        gst_buffer_unref(buffer);
        g_static_mutex_unlock (&mutex);
    }

    void FarsightChannel::HandleAudioData(u8* data, int size, int rate, int width, int channels)
    {
        boost::mutex::scoped_lock lock(audio_queue_mutex_);
        int audio_buffer_size = AUDIO_BUFFER_SIZE_MS * channels * width / 8 * rate / 1000;
        if (audio_buffer_size > AUDIO_BUFFER_MAX_SIZE)
            audio_buffer_size = AUDIO_BUFFER_MAX_SIZE;

        int available_buffer_size = audio_buffer_size - available_audio_data_length_;
        if (width > 8 && available_buffer_size%2 == 1)
            available_buffer_size--; // Only full samples to audio buffer
        if (available_buffer_size < size)
        {
            emit AudioBufferOverflow( size - available_buffer_size );
            size = available_buffer_size;
        }

        if (write_cursor_ + size >= AUDIO_BUFFER_MAX_SIZE)
        {
            // The data must be copied in to two sections
            int size_at_end_of_the_buffer = AUDIO_BUFFER_MAX_SIZE - write_cursor_;
            int size_at_begin_of_the_buffer = size - size_at_end_of_the_buffer;

            memcpy(audio_buffer_ + write_cursor_, data, size_at_end_of_the_buffer);
            write_cursor_ = (write_cursor_ + size_at_end_of_the_buffer) % AUDIO_BUFFER_MAX_SIZE;
            assert( write_cursor_ == 0 );

            memcpy(audio_buffer_ + write_cursor_, data + size_at_end_of_the_buffer, size_at_begin_of_the_buffer);
            write_cursor_ = (write_cursor_ + size_at_begin_of_the_buffer ) % AUDIO_BUFFER_MAX_SIZE;
        }
        else
        {
            memcpy(audio_buffer_ + write_cursor_, data, size);
            write_cursor_ = (write_cursor_ + size) % AUDIO_BUFFER_MAX_SIZE;
        }

        received_sample_rate_ = rate;
        received_sample_width_ = width;
        received_channel_count_ = channels;

        available_audio_data_length_ += size;
        emit AudioDataAvailable(available_audio_data_length_);
    }

    GstElement* FarsightChannel::setUpElement(const QString &element_name)
    {
        GstElement* element = gst_element_factory_make(element_name.toStdString().c_str(), NULL);
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
        if(value<0||value>1)
        {
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
        try
        {
            if(self->tf_channel_ == NULL) 
            {
                LogWarning("CommunicationModule: receiving bus message when tf_channel_ is NULL");
                return FALSE;
            }
            tf_channel_bus_message(self->tf_channel_, message);
            return TRUE;
        } catch(...)
        {
            LogWarning("CommunicationModule: passing gstreamer bus message to telepathy-farsight failed");
            return FALSE;
        }
    }

    void FarsightChannel::onClosed(TfChannel *tfChannel, FarsightChannel *self)
    {
        self->status_ = StatusDisconnected;
        emit self->StatusChanged(self->status_);
    }

    void FarsightChannel::onSessionCreated(TfChannel *tfChannel, FsConference *conference, FsParticipant *participant, FarsightChannel *self)
    {
        self->bus_watch_ = gst_bus_add_watch(self->bus_, (GstBusFunc) &FarsightChannel::busWatch, self);
        gst_bin_add(GST_BIN(self->pipeline_), GST_ELEMENT(conference));
        gst_element_set_state(GST_ELEMENT(conference), GST_STATE_PLAYING);
    }

    void FarsightChannel::onStreamCreated(TfChannel *tfChannel, TfStream *stream, FarsightChannel *self)
    {
        guint media_type;
        GstPad *sink;

        g_signal_connect(stream, "src-pad-added", G_CALLBACK(&FarsightChannel::onSrcPadAdded), self);
        g_signal_connect(stream, "request-resource", G_CALLBACK(&FarsightChannel::onRequestResource), NULL);

        g_object_get(stream, "media-type", &media_type, "sink-pad", &sink, NULL);

        GstPad *pad;

        switch (media_type)
        {
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

    void FarsightChannel::onSrcPadAdded(TfStream *stream, GstPad *src_pad, FsCodec *codec, FarsightChannel *self)
    {
        self->LinkIncomingSourcePad(stream, src_pad, codec);
    }

    void FarsightChannel::LinkIncomingSourcePad(TfStream *stream, GstPad *src_pad, FsCodec *codec)
    {           
        incoming_video_widget_mutex_.lock();

        // todo: Check if source pad is already linked!
        gint clock_rate = codec->clock_rate;
        audio_stream_in_clock_rate_ = clock_rate;
        gint channel_count = codec->channels;

        guint media_type;
        g_object_get(stream, "media-type", &media_type, NULL);

        GstPad *output_pad;
        GstElement *output_element = 0;

        bool sink_already_linked = false;

        switch (media_type)
        {
            case TP_MEDIA_STREAM_TYPE_AUDIO:
            {
                output_element = audio_playback_bin_;
                if (audio_in_src_pad_)
                    sink_already_linked = true;
                LogInfo("Got pad for incoming AUDIO stream.");
                break;
            }
            case TP_MEDIA_STREAM_TYPE_VIDEO:
            {
                if (!video_supported_)
                {
                    LogInfo("Got incoming VIDEO stream but ignore that because lack of video support.");
                    incoming_video_widget_mutex_.unlock();
                    return;
                }

                output_element = received_video_playback_element_;
                if (video_in_src_pad_)
                    sink_already_linked = true;
                LogDebug("Got pad for incoming VIDEO stream.");
                break;
            }
            default:
            {
                Q_ASSERT(false);
            }
        }

        if (sink_already_linked)
        {
            LogInfo("FarsightChannel: another Src pad added with same type.");
        }
        else
        {
            if (!gst_bin_add(GST_BIN(pipeline_), output_element))
            {
                LogWarning("Cannot and output element to GStreamer pipeline!");
            }
        }

        output_pad = gst_element_get_static_pad(output_element, "sink");
        if (!output_pad)
        {
            LogError("Cannot get sink pad from output element");
        }
        switch (media_type)
        {
            case TP_MEDIA_STREAM_TYPE_AUDIO:
            {
                if (audio_in_src_pad_)
                {
                    gst_pad_unlink(audio_in_src_pad_, output_pad);
                }
                audio_in_src_pad_ = src_pad;
                break;
            }
            case TP_MEDIA_STREAM_TYPE_VIDEO:
            {
                if (video_in_src_pad_)
                {
                    gst_pad_unlink(video_in_src_pad_, output_pad);
                }
                video_in_src_pad_ = src_pad;
                break;
            }
        }

        if (gst_pad_link(src_pad, output_pad) != GST_PAD_LINK_OK)
        {
            LogWarning("Cannot link audio src to output element.");
        }
        gst_element_set_state(output_element, GST_STATE_PLAYING);

        incoming_video_widget_mutex_.unlock();

        status_ = StatusConnected;
        emit StatusChanged(status_);
        switch (media_type)
        {
            case TP_MEDIA_STREAM_TYPE_AUDIO:
                emit AudioStreamReceived();
                break;
            case TP_MEDIA_STREAM_TYPE_VIDEO:
                emit VideoStreamReceived();
                break;
        }
    }

    gboolean FarsightChannel::onRequestResource(TfStream *stream, guint direction, gpointer data)
    {
        LogDebug("resource request");
        return TRUE;
    }

    VideoWidget* FarsightChannel::GetLocallyCapturedVideoWidget()
    {
        if (!video_supported_)
            return 0;
        return locally_captured_video_widget_;
    }

    VideoWidget* FarsightChannel::GetReceivedVideoWidget()
    {
        if (!video_supported_)
            return 0;
        incoming_video_widget_mutex_.lock();
        if (!video_in_src_pad_) // FIXME: Make this thread safe and use more sane variable
        {
            incoming_video_widget_mutex_.unlock();
            return 0;
        }
        incoming_video_widget_mutex_.unlock();

        return received_video_widget_;
    }

    int FarsightChannel::GetSampleRate() const
    {
        return received_sample_rate_;
    }

    int FarsightChannel::GetSampleWidth() const
    {
        return received_sample_width_;
    }
    
    int FarsightChannel::GetChannelCount() const
    {
        return received_channel_count_;
    }
    
    int FarsightChannel::GetAvailableAudioDataLength() const
    {
        return available_audio_data_length_;
    }

    int FarsightChannel::GetAudioData(u8* buffer, int max)
    {
        boost::mutex::scoped_lock lock(audio_queue_mutex_);
        
        int size = 0;
        if (available_audio_data_length_ <= max)
            size = available_audio_data_length_;
        else
            size = max;

        if (read_cursor_ + size >= AUDIO_BUFFER_MAX_SIZE)
        {
            // we have to copy audio data from two segments
            int size_at_end_of_buffer = AUDIO_BUFFER_MAX_SIZE - read_cursor_; 
            int size_at_begin_of_buffer = size - size_at_end_of_buffer;
            memcpy(buffer , audio_buffer_ + read_cursor_, size_at_end_of_buffer);
            read_cursor_ = (read_cursor_ + size_at_end_of_buffer) % AUDIO_BUFFER_MAX_SIZE;
            assert( read_cursor_ == 0 );

            memcpy(buffer + size_at_end_of_buffer, audio_buffer_ + read_cursor_, size_at_begin_of_buffer);
            read_cursor_ = (read_cursor_ + size_at_begin_of_buffer) % AUDIO_BUFFER_MAX_SIZE;
        }
        else
        {
            memcpy(buffer , audio_buffer_ + read_cursor_, size);
            read_cursor_ = (read_cursor_ +size ) % AUDIO_BUFFER_MAX_SIZE;
        }
        available_audio_data_length_ -= size;
        return size;
    }

} // end of namespace: TelepathyIM
