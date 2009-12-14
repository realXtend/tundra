// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VideoWidget.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QThread>
#include <QVariant>
#include <QObject>
#include <gst/interfaces/xoverlay.h>

namespace TelepathyIM
{
    VideoWidget::VideoWidget(GstBus *bus,  QWidget *parent) 
        : Communication::VideoWidgetInterface(parent), 
          bus_((GstBus *) gst_object_ref(bus)), 
          overlay_(0), 
          sink_(0)
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
        window_id_ = winId();
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

            QObject *obj = dynamic_cast<QObject *>(self);
            QMetaObject::invokeMethod(obj, "SetOverlay", Qt::QueuedConnection);
            QMetaObject::invokeMethod(obj, "WindowExposed", Qt::QueuedConnection);
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
            QObject *obj = dynamic_cast<QObject *>(self);
            QMetaObject::invokeMethod(obj, "SetOverlay", Qt::QueuedConnection);
        }
    }

    void VideoWidget::showEvent(QShowEvent *showEvent)
    {
        //setAttribute(Qt::WA_NoSystemBackground, true);
        //setAttribute(Qt::WA_PaintOnScreen, true);
        QWidget::showEvent(showEvent);

        SetOverlay();
    }

    void VideoWidget::SetOverlay()
    {
        if (overlay_ && GST_IS_X_OVERLAY(overlay_))
        {
            //WId windowId = winId();
            QApplication::syncX();
            gst_x_overlay_set_xwindow_id( GST_X_OVERLAY(overlay_), (gulong) window_id_);
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

    bool VideoWidget::VideoAvailable()
    {
        if (this->overlay_)
            return true;
        else
            return false;
    }

    void VideoWidget::SetParentWidget(QWidget &parent)
    {
        // todo: IMPLEMENT

        window_id_ = parent.winId();
        SetOverlay();
    }

}
