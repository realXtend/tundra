// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VideoWidget.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QThread>
#include <QVariant>
#include <QObject>
#include <QDebug>

#include <gst/interfaces/xoverlay.h>

#ifdef Q_WS_X11
extern void qt_x11_set_global_double_buffer(bool);
#endif

namespace TelepathyIM
{
    VideoWidget::VideoWidget(GstBus *bus,  QWidget *parent, const QString &name) 
        : Communication::VideoWidgetInterface(parent), 
          bus_((GstBus *) gst_object_ref(bus)), 
          video_overlay_(0), 
          video_playback_element_(0),
          video_bin_(0),
          name_(name),
          window_id_(0),
          on_element_added_g_signal_(0),
          on_sync_message_g_signal_(0)

    {
        qDebug() << "VideoWidget " << name << " INIT STARTED";
        setWindowTitle(name);

        // Element notifier init
        notifier_ = fs_element_added_notifier_new();
        on_element_added_g_signal_ = g_signal_connect(notifier_, "element-added", G_CALLBACK(&VideoWidget::OnElementAdded), this);

// UNIX -> autovideosink
#ifdef Q_WS_X11

        qt_x11_set_global_double_buffer(false);

        video_playback_element_ = gst_element_factory_make("autovideosink", name.toStdString().c_str());
        gst_object_ref(video_playback_element_);
        gst_object_sink(video_playback_element_);

        fs_element_added_notifier_add(notifier_, GST_BIN(video_playback_element_));

#endif

// WINDOWS -> autovideosink will chose one of there: glimagesink (best), directdrawsink (possible buffer errors), dshowvideosink (possible buffer errors)
#ifdef Q_WS_WIN

        video_playback_element_ = gst_element_factory_make("glimagesink", 0);
        if (!video_playback_element_)
        {
            qDebug() << "VideoWidget " << name << " CANNOT CREATE video_playback_element_";
            return;
        }

		//if (!video_playback_element_)
		//{
		//	fs_element_added_notifier_add(notifier_, GST_BIN(video_playback_element_));
		//	gst_object_ref(video_playback_element_);
		//	gst_object_sink(video_playback_element_);
		//}
		//else
		//{
			// Video bin init
			const QString video_bin_name = "video_bin_for_" + name;
			video_bin_ = gst_bin_new(video_bin_name.toStdString().c_str());
			if (!video_bin_)
			{
				qDebug() << "VideoWidget " << name << " CANNOT CREATE video_bin_";
				return;
			}

			// Add playback element to video bin
			gst_bin_add(GST_BIN(video_bin_), video_playback_element_);

			// Pad inits
			GstPad *static_sink_pad = gst_element_get_static_pad(video_playback_element_, "sink");
			GstPad *sink_ghost_pad = gst_ghost_pad_new("sink", static_sink_pad);

			// Add bad to video bin
			gst_element_add_pad(GST_ELEMENT(video_bin_), sink_ghost_pad);
			gst_object_unref(G_OBJECT(static_sink_pad));
			gst_object_ref(video_bin_);
			gst_object_sink(video_bin_);

			fs_element_added_notifier_add(notifier_, GST_BIN(video_bin_));
		//}

#endif
        
        gst_bus_enable_sync_message_emission(bus_);
        on_sync_message_g_signal_ = g_signal_connect(bus_, "sync-message", G_CALLBACK(&VideoWidget::OnSyncMessage), this);

        qDebug() << "VideoWidget " << name << " INIT COMPLETE";

        // QWidget properties
        QPalette palette;
        palette.setColor(QPalette::Background, Qt::black);
        palette.setColor(QPalette::Window, Qt::black);
        setPalette(palette);
        setAutoFillBackground(true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_PaintOnScreen, true);
        resize(322, 240);
        setMinimumSize(322, 240);
    }

    VideoWidget::~VideoWidget()
    {
        if (notifier_)
        {
            fs_element_added_notifier_remove(notifier_, GST_BIN(video_playback_element_));
            g_signal_handler_disconnect(notifier_, on_element_added_g_signal_);
        }
        if (bus_)
            g_signal_handler_disconnect(bus_, on_sync_message_g_signal_);

        if (bus_)
        {
            g_object_unref(bus_);
            bus_ = 0;
        }
        if (video_playback_element_)
        {
            //if (GST_BIN(video_playback_element_))
            //    g_object_unref(video_playback_element_);
            video_playback_element_ = 0;
        }
        if (video_bin_)
        {
            g_object_unref(video_bin_);
            video_bin_ = 0;
        }
        // todo unconnect signals ?
    }

    void VideoWidget::OnElementAdded(FsElementAddedNotifier *notifier, GstBin *bin, GstElement *element, VideoWidget *self)
    {
        // If element implements GST_X_OVERLAY interface, set local video_overlay_ to element
        // If true element = the current video sink
        if (!self->video_overlay_ && GST_IS_X_OVERLAY(element))
        {
            qDebug() << self->name_ << " >> element-added CALLBACK >> Got overlay element, storing";
            self->video_overlay_ = element;
            QMetaObject::invokeMethod(self, "WindowExposed", Qt::QueuedConnection);
        }

        // If element has property force-aspect-ratio set it to true
        // If true element = the current video sink
        if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "force-aspect-ratio"))
        {
            qDebug() << self->name_ << " >> element-added CALLBACK >> found 'force-aspect-ratio' from element";
            g_object_set(G_OBJECT(element), "force-aspect-ratio", TRUE, NULL);
        }
    }

    void VideoWidget::OnSyncMessage(GstBus *bus, GstMessage *message, VideoWidget *self)
    {
        // Return if we are not interested in the message content
        if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
            return;
        if (GST_MESSAGE_SRC(message) != (GstObject *)self->video_overlay_)
            return;

        // If message is about preparing xwindow id its from the current video sink
        // and we want to set our own qt widget window id where we want to render video
        const GstStructure *s = gst_message_get_structure(message);
        if (gst_structure_has_name(s, "prepare-xwindow-id") && self->video_overlay_)
        {
            qDebug() << self->name_ << " >> sync-message CALLBACK >> found 'prepare-xwindow-id' from GstMessage";
            QMetaObject::invokeMethod(self, "SetOverlay", Qt::QueuedConnection);
        }
    }

    void VideoWidget::showEvent(QShowEvent *showEvent)
    {
        // Override showEvent so we can set QWidget attributes and set overlay
        qDebug() << name_ << " >> QWidget::showEvent() override called";
        QWidget::showEvent(showEvent);
        SetOverlay();
    }

    void VideoWidget::SetOverlay()
    {
        if (video_overlay_ && GST_IS_X_OVERLAY(video_overlay_))
        {
            // Get window id from this widget and set it for video sink
            // so it renders to our widget id and does not open separate window (that is the default behaviour)
            qDebug() << name_ << " >> SetOverlay() called";
			window_id_ = winId();
            if (window_id_)
            {
                qDebug() << name_ << " >> Giving x overlay widgets window id " << window_id_;
                #ifdef Q_WS_X11
                    QApplication::syncX();
                #endif
                gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(video_overlay_), (gulong)window_id_);
            }
        }
        WindowExposed();
    }

    void VideoWidget::WindowExposed()
    {
        if (video_overlay_ && GST_IS_X_OVERLAY(video_overlay_))
        {
            // Expose the overlay (some sort of update)
            qDebug() << name_ << " >> WindowExposed() called";
            #ifdef Q_WS_X11
                QApplication::syncX();
            #endif
            gst_x_overlay_expose(GST_X_OVERLAY(video_overlay_));
        }
    }

	GstElement *VideoWidget::GetVideoPlaybackElement() const
	{
		if (video_bin_) 
			return video_bin_; 
		else if (video_playback_element_)
			return video_playback_element_;
		else
			return 0;
	}

    bool VideoWidget::VideoAvailable()
    {
        if (video_overlay_)
            return true;
        else
            return false;
    }

}
