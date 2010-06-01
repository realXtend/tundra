// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_VideoSessionInterface_h
#define incl_Comm_VideoSessionInterface_h

#include <QObject>

namespace Communication
{
    class VideoPlaybackWidgetInterface;

    /**
     * NOT IN USE CURRENTLY: VoiceSessionInterface have video functionality
     *
     */
    class VideoSessionInterface : public QObject
    {
        Q_OBJECT
    public:
        enum State { STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR};
        virtual State GetState() const = 0;

        virtual void Close() = 0;
        
        virtual VideoPlaybackWidgetInterface* GetRemoteVideo() = 0;
        virtual VideoPlaybackWidgetInterface* GetLocalVideo() = 0;
    };

}

#endif

