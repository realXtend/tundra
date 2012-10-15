// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MessageFilter.h"

class InterestManager;

class RayVisibilityFilter : public MessageFilter
{

public:

    RayVisibilityFilter(InterestManager *im, int r, int interval, bool enabled);

    ~RayVisibilityFilter();

    bool Filter(IMParameters params);

private:

    InterestManager *im_;
    int range_;
    int raycastinterval_;
};
