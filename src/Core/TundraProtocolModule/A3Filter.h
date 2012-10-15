// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MessageFilter.h"

class InterestManager;

class A3Filter : public MessageFilter
{

public:

    A3Filter(InterestManager *im, int criticalrange, int maxrange, int updateinterval, bool enabled);

    ~A3Filter();

    bool Filter(IMParameters params);

private:

    MessageFilter *euclideandistance_;
    MessageFilter *relevance_;
};
