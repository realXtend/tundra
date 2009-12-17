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

namespace TelepathyIM
{
    VideoWidget::VideoWidget(GstBus *bus,  QWidget *parent, const QString &name) 
        : Communication::VideoWidgetInterface(parent), 
          bus_((GstBus *) gst_object_ref(bus)), 
          video_overlay_(0), 
          video_playback_element_(0),
          name_(name)
    {
        qDebug() << "VideoWidget '" << name << "' INIT STARTED";
        gst_object_ref(bus_);
        gst_object_sink(bus_);

        // Element notifier init
        notifier_ = fs_element_added_notifier_new();
        g_signal_connect(notifier_, "element-added", G_CALLBACK(&VideoWidget::OnElementAdded), this);

        // Video sink init
        video_playback_element_ = gst_element_factory_make("glimagesink", name.toStdString().c_str());
        gst_object_ref(video_playback_element_);
        gst_object_sink(video_playback_element_);

        //////////// NEW CODE START ////////////

        //// Video bin init
        //const QString video_bin_name = "video_bin_for_" + name;
        //GstElement *video_bin = gst_bin_new(video_bin_name.toStdString().c_str());
        //// Add playback element to video bin
        //gst_bin_add(GST_BIN(video_bin), video_playback_element_);
        //// Link to bin 
        //gst_element_link(video_bin, video_playback_element_); // <-- MATTIKU CHECK PLEASE

        //// Pad inits
        //GstPad *static_sink_pad = gst_element_get_static_pad(video_playback_element_, "sink"); // <-- MATTIKU CHECK PLEASE
        //GstPad *sink_ghost_pad = gst_ghost_pad_new("sink", static_sink_pad); // <-- MATTIKU CHECK PLEASE
        //// Add bad to video bin
        //gst_element_add_pad(video_bin, sink_ghost_pad); // <-- MATTIKU CHECK PLEASE

        //fs_element_added_notifier_add(notifier_, GST_BIN(video_bin)); // <-- THIS WORKS NOW BECAUSE VIDEO IS INSIDE A BIN ELEMENT
        
        ///////////// NEW CODE END ////////////

        fs_element_added_notifier_add(notifier_, GST_BIN(video_playback_element_));
        
        gst_bus_enable_sync_message_emission(bus_);
        g_signal_connect(bus_, "sync-message", G_CALLBACK(&VideoWidget::OnSyncMessage), this);

        qDebug() << "VideoWidget '" << name << "' INIT COMPLETE";
    }

    VideoWidget::~VideoWidget()
    {
        g_object_unref(bus_);
        g_object_unref(video_playback_element_);
    }

    void VideoWidget::OnElementAdded(FsElementAddedNotifier *notifier, GstBin *bin, GstElement *element, VideoWidget *self)
    {
        if (!self->video_overlay_ && GST_IS_X_OVERLAY(element))
        {
            qDebug() << self->name_ << " >> element-added CALLBACK >> Got overlay element, storing";
            self->video_overlay_ = element;
            QMetaObject::invokeMethod(self, "WindowExposed", Qt::QueuedConnection);
        }

        if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "force-aspect-ratio"))
        {
            qDebug() << self->name_ << " >> element-added CALLBACK >> found 'force-aspect-ratio' from element";
            g_object_set(G_OBJECT(element), "force-aspect-ratio", TRUE, NULL);
        }
    }

    void VideoWidget::OnSyncMessage(GstBus *bus, GstMessage *message, VideoWidget *self)
    {
        if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
            return;

        if (GST_MESSAGE_SRC(message) != (GstObject *)self->video_overlay_)
            return;

        const GstStructure *s = gst_message_get_structure(message);
        if (gst_structure_has_name(s, "prepare-xwindow-id") && self->video_overlay_)
        {
            qDebug() << self->name_ << " >> sync-message CALLBACK >> found 'prepare-xwindow-id' from GstMessage";
            QMetaObject::invokeMethod(self, "SetOverlay", Qt::QueuedConnection);
        }
    }

    void VideoWidget::showEvent(QShowEvent *showEvent)
    {
        qDebug() << name_ << " >> QWidget::showEvent() override called";
        QPalette palette;
        palette.setColor(QPalette::Background, QColor(0,255,0,125));
        setPalette(palette);

        setAutoFillBackground(true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_PaintOnScreen, true);

        SetOverlay();
        QWidget::showEvent(showEvent);
    }

    void VideoWidget::SetOverlay()
    {
        if (video_overlay_ && GST_IS_X_OVERLAY(video_overlay_))
        {
            qDebug() << name_ << " >> SetOverlay() called";
            window_id_ = winId();
            if (window_id_)
            {
                qDebug() << name_ << " >> Window id se to " << window_id_ << " window_id_->unused = " << window_id_->unused;
                QApplication::syncX();
                gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(video_overlay_), (gulong)window_id_);
            }
        }
        WindowExposed();
    }

    void VideoWidget::WindowExposed()
    {
        if (video_overlay_ && GST_IS_X_OVERLAY(video_overlay_))
        {
            qDebug() << name_ << " >> WindowExposed() called";
            QApplication::syncX();
            gst_x_overlay_expose(GST_X_OVERLAY(video_overlay_));
        }
    }

    bool VideoWidget::VideoAvailable()
    {
        if (this->video_overlay_)
            return true;
        else
            return false;
    }

    void VideoWidget::SetParentWidget(QWidget &parent)
    {
        // todo: IMPLEMENT
        //window_id_ = parent.winId();
        //SetOverlay();
    }

}
