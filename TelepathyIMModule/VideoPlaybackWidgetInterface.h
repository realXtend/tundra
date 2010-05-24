// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_VideoPlaybackWidgetInterface_h
#define incl_Comm_VideoPlaybackWidgetInterface_h

#include <QWidget>

namespace Communication
{
    /**
     *  Video playback widget
     *
     */
    class VideoPlaybackWidgetInterface : public QWidget
    {
    
    public:
        explicit VideoPlaybackWidgetInterface(QWidget *parent) : QWidget(parent) {}

        //! @return true if video source is available. Otherwise return false
        virtual bool VideoAvailable() = 0;
    };
}

#endif

