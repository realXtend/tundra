// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VideoWidget.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QThread>
#include <QVariant>
#include <gst/interfaces/xoverlay.h>

namespace TelepathyIM
{

    //QWidget(bus,parent)
    VideoWidget::VideoWidget(GstBus *bus,  QWidget *parent) : QWidget(parent), bus_((GstBus *) gst_object_ref(bus)), overlay_(0), sink_(0)
    {
        notifier_ = fs_element_added_notifier_new();
        g_signal_connect(notifier_, "element-added", G_CALLBACK(&VideoWidget::OnElementAdded), this);


        sink_ = gst_element_factory_make("glimagesink", NULL);   //TODO: Get element name as argument
        gst_object_ref(sink_);
        gst_object_sink(sink_);

        fs_element_added_notifier_add(notifier_, GST_BIN(sink_));
        gst_bus_enable_sync_message_emission(bus_);

        g_signal_connect(bus_, "sync-message", G_CALLBACK(&VideoWidget::OnSyncMessage), this);


        QPalette palette;
        palette.setColor(QPalette::Background, Qt::black);
        setPalette(palette);
        setAutoFillBackground(true);
    }

    VideoWidget::~VideoWidget()
    {
        g_object_unref(bus_);
        g_object_unref(sink_);
    }

    void VideoWidget::OnElementAdded(FsElementAddedNotifier *notifier, GstBin *bin, GstElement *element, VideoWidget *self)
    {
        if (!self->overlay_ && GST_IS_X_OVERLAY(element))
        {
            self->overlay_ = element;
            QMetaObject::invokeMethod(self, "WindowExposed", Qt::QueuedConnection);
        }

        if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "force-aspect-ratio"))
        {
            g_object_set(G_OBJECT(element), "force-aspect-ratio", TRUE, NULL);
        }
    }

    void VideoWidget::OnSyncMessage(GstBus *bus, GstMessage *message, VideoWidget *self)
    {
        if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
        {
            return;
        }

        if (GST_MESSAGE_SRC(message) != (GstObject *) self->overlay_)
        {
            return;
        }

        const GstStructure *s = gst_message_get_structure (message);

        if (gst_structure_has_name(s, "prepare-xwindow-id") && self->overlay_)
        {
            QMetaObject::invokeMethod(self, "SetOverlay", Qt::QueuedConnection);
        }
    }

    GstElement* VideoWidget::GetVideoSink() const
    {
        return sink_;
    }

    bool VideoWidget::EventFilter(QEvent *ev)
    {
        if (ev->type() == QEvent::Show)
        {
            setAttribute(Qt::WA_NoSystemBackground, true);
            setAttribute(Qt::WA_PaintOnScreen, true);
            SetOverlay();
        }
        return false;
    }

    void VideoWidget::SetOverlay()
    {
        if (overlay_ && GST_IS_X_OVERLAY(overlay_))
        {
            WId windowId = winId();
            //gulong guWindowId = (gulong) windowId;
            QApplication::syncX();
            gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(overlay_),
                    (gulong) windowId);
        }
        WindowExposed();
    }

    void VideoWidget::WindowExposed()
    {
        QApplication::syncX();
        if (overlay_ && GST_IS_X_OVERLAY(overlay_))
        {
            gst_x_overlay_expose(GST_X_OVERLAY(overlay_));
        }
    }
}
