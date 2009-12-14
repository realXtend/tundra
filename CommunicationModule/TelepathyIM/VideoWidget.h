#ifndef incl_Communication_TelepathyIM_VideoWidget_h
#define incl_Communication_TelepathyIM_VideoWidget_h

#include <QWidget>
#include <gst/gst.h>
#include <gst/farsight/fs-element-added-notifier.h>

#include "../interface.h"
              
namespace TelepathyIM
{
    /**
     *  Based on call example of telepathy-qt4 library
     *  QWidget for video playback. Created by FarsightChannel object and used by communication UI widgets.
     */
    class VideoWidget : public Communication::VideoWidgetInterface
    {

    Q_OBJECT
    
    public: 
        VideoWidget(GstBus *bus, QWidget *parent = 0);
        virtual ~VideoWidget();

        //! Getters
        virtual bool VideoAvailable();
        GstElement *GetVideoSink() const { return sink_; }

        //! Setters
        virtual void SetParentWidget(QWidget &parent);

        //! Glib callbacks
        static void OnElementAdded(FsElementAddedNotifier *notifier, GstBin *bin, GstElement *element, VideoWidget *self);
        static void OnSyncMessage(GstBus *bus, GstMessage *message, VideoWidget *self);
    
    protected:
        void showEvent(QShowEvent *showEvent);

    private slots:
        void SetOverlay();
        void WindowExposed();

    private:
        GstBus *bus_;
        FsElementAddedNotifier *notifier_;
        GstElement *sink_;
        GstElement *overlay_;
        WId window_id_; 
    };

} // End of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VideoWidget_h
