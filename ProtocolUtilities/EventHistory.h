// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_ProtocolUtilities_EventHistory_h
#define incl_ProtocolUtilities_EventHistory_h

#include <vector>
#include <utility>
#include "HighPerfClock.h"

/// Maintains a timestamped history of events that have occurred. Bounds the maximum memory usage to the N most recent entries.
class EventHistory
{
    std::vector<std::pair<tick_t, double> > records_;
    size_t maxHistorySize_;

public:
    explicit EventHistory(size_t maxHistorySize)
    :maxHistorySize_(maxHistorySize)
    {
    }

    void InsertRecord(double record)
    {
        tick_t time = GetCurrentClockTime();
        records_.push_back(std::make_pair(time, record));
        if (records_.size() > maxHistorySize_)
            records_.erase(records_.begin());
    }

    static double SmoothedAvgPerSecond(const std::vector<double> &dstAccum, double bucketSize, double coeff)
    {
        double factor = 1.0;
        double total = 0.0;
        double denom = 0.0;
        int i = dstAccum.size()-1;
        while(factor >= 0.01 && i >= 0)
        {
            total += dstAccum[i] / bucketSize * factor;
            denom += factor;
            factor *= coeff;
        }
        return total / denom;
    }

    void OutputBucketedAccumulated(std::vector<double> &dstAccum, size_t numEntries, double bucketSize, std::vector<double> *dstOccurCount)
    {
        dstAccum.clear();
        dstAccum.resize(numEntries, 0);
        if (dstOccurCount)
        {
            dstOccurCount->clear();
            dstOccurCount->resize(numEntries, 0);
        }
        tick_t time = GetCurrentClockTime();
        tick_t modulus = (tick_t)(GetCurrentClockFreq() * bucketSize);
        time -= time % modulus;

        for(size_t i = 0; i < records_.size(); ++i)
        {
            tick_t age = time - records_[i].first;
            if (age >= (tick_t)(-1) >> 1)
                continue;

            int idx = age / (double)GetCurrentClockFreq() / bucketSize;
            if (idx < 0 || idx >= numEntries)
                continue;
            dstAccum[dstAccum.size()-1-idx] += records_[i].second;
            if (dstOccurCount)
                (*dstOccurCount)[dstAccum.size()-1-idx]++;
        }
    }
};

#endif
