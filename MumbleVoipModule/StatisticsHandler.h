// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_StatisticsHandler_h
#define incl_MumbleVoipModule_StatisticsHandler_h

#include "ui_VoiceSettings.h"
#include <QObject>
#include <QTimer>

namespace MumbleVoip
{
    /// Mumble server connection statistics handler
    class StatisticsHandler : public QObject
    {
        Q_OBJECT
    public:
        StatisticsHandler(int time_frame_ms);
        virtual ~StatisticsHandler();
            
        void NotifyBytesSent(int bytes);
        void NotifyBytesReceived(int bytes);
        int GetAverageBandwidthIn() const;
        int GetAverageBandwidthOut() const;
        void SetTimeFrameMs(int time_frame_ms);
        int GetTimeFrameMs() const;
    signals:
        void StatisticsUpdated();
    private slots:
        void UpdateStatistics();
    private:
        int time_frame_ms_;
        QTimer timer_;
        int average_bandwidth_in_;
        int average_bandwidth_out_;
        int bytes_in_;
        int bytes_out_;
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_StatisticsHandler_h
