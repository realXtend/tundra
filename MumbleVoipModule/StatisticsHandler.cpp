// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "StatisticsHandler.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    StatisticsHandler::StatisticsHandler(int time_frame_ms) : 
        time_frame_ms_(time_frame_ms),
        bytes_in_(0),
        bytes_out_(0)
    {
        connect(&timer_, SIGNAL(timeout()), this, SLOT(UpdateStatistics()));
        if (time_frame_ms_ != 0)
        {
            timer_.setInterval(time_frame_ms_);
            timer_.start();
        }
    }

    StatisticsHandler::~StatisticsHandler()
    {

    }
            
    void StatisticsHandler::NotifyBytesSent(int bytes)
    {
        bytes_out_ += bytes;
    }

    void StatisticsHandler::NotifyBytesReceived(int bytes)
    {
        bytes_in_ += bytes;
    }

    int StatisticsHandler::GetAverageBandwidthIn() const
    {
        return average_bandwidth_in_;
    }

    int StatisticsHandler::GetAverageBandwidthOut() const
    {
        return average_bandwidth_out_;
    }

    void StatisticsHandler::SetTimeFrameMs(int time_frame_ms)
    {
        if (time_frame_ms_ != 0)
        {
            time_frame_ms_ = time_frame_ms;
            timer_.setInterval(time_frame_ms_);
            timer_.start();
        }
        else
        {
            timer_.stop();
        }

    }

    int StatisticsHandler::GetTimeFrameMs() const
    {
        return time_frame_ms_;
    }

    void StatisticsHandler::UpdateStatistics()
    {
        average_bandwidth_in_ = 1000*bytes_in_ / time_frame_ms_;
        average_bandwidth_out_ = 1000*bytes_out_ / time_frame_ms_;
        bytes_in_ = 0;
        bytes_out_ = 0;
        emit StatisticsUpdated();
    }
}
