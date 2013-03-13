// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MessageFilter.h"

class InterestManager;

class EA3Filter : public MessageFilter
{

public:

    EA3Filter(InterestManager *im, int criticalrange, int maxrange, int raycastinterval, int updateinterval, bool enabled);

    ~EA3Filter();

    bool Filter(const IMParameters& params);

    QString ToString();

private:

    InterestManager *im_;
    MessageFilter *euclideandistance_;
    MessageFilter *rayvisibility_;
    MessageFilter *relevance_;
};
