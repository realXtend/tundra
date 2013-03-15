// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MessageFilter.h"

class InterestManager;

class EuclideanDistanceFilter : public MessageFilter
{

public:

    EuclideanDistanceFilter(InterestManager *im, int criticaldistance, bool enabled);

    ~EuclideanDistanceFilter() {}

    bool Filter(const IMParameters& params);

    QString ToString();

private:

    InterestManager *im_;
    int radius_;
};
