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
    class VideoWidget : public Communication::VideoPlaybackWidgetInterface
    {

    Q_OBJECT
    
    public: 
        VideoWidget(GstBus *bus, QWidget *parent = 0, const QString &name = QString("video_widget"));
        virtual ~VideoWidget();

        //! Getters
        virtual bool VideoAvailable();
        GstElement *GetVideoPlaybackElement() const;

        //! Glib callbacks
        static void OnElementAdded(FsElementAddedNotifier *notifier, GstBin *bin, GstElement *element, VideoWidget *self);
        static void OnSyncMessage(GstBus *bus, GstMessage *message, VideoWidget *self);
    
    protected:
        void showEvent(QShowEvent *showEvent);
		void closeEvent(QCloseEvent *closeEvent);

    private slots:
        void SetOverlay();
        void WindowExposed();

    private:
        //! Gst Fs variables
        GstBus *bus_;
        GstElement *video_bin_;
        GstElement *video_playback_element_;
        GstElement *video_overlay_;
        FsElementAddedNotifier *notifier_;
        gulong on_element_added_g_signal_;
        gulong on_sync_message_g_signal_;

        
        //! Qt variables
        WId window_id_; 
        QString name_;
    };

} // End of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VideoWidget_h
