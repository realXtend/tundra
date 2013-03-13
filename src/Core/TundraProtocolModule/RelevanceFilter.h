// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MessageFilter.h"

class InterestManager;

class RelevanceFilter : public MessageFilter
{

public:

    RelevanceFilter(InterestManager *im, int r, int cr, int interval, bool enabled);

    ~RelevanceFilter() {}

    bool Filter(const IMParameters& params);

    QString ToString();

private:

    InterestManager *im_;
    int critical_range_;
    int range_;
    int updateinterval_;
};
