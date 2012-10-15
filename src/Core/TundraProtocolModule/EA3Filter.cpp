// For conditions of distribution and use, see copyright notice in LICENSE

#include "InterestManager.h"
#include "EA3Filter.h"
#include "EuclideanDistanceFilter.h"
#include "RayVisibilityFilter.h"
#include "RelevanceFilter.h"

EA3Filter::EA3Filter(InterestManager *im, int criticalrange, int maxrange, int raycastinterval, int updateinterval, bool enabled) :
    MessageFilter(EA3, enabled)
{
    euclideandistance_ = new EuclideanDistanceFilter(im, criticalrange, true);
    rayvisibility_ = new RayVisibilityFilter(im, maxrange, raycastinterval, true);
    relevance_ = new RelevanceFilter(im, maxrange, criticalrange, updateinterval, true);
}

EA3Filter::~EA3Filter()
{
    delete euclideandistance_;
    delete rayvisibility_;
    delete relevance_;
}

bool EA3Filter::Filter(IMParameters params)
{
    bool accepted = true;

    if(enabled_)
    {
        accepted = euclideandistance_->Filter(params);

        if(accepted)
            return true;

        if(params.dot >= 0)
            accepted = rayvisibility_->Filter(params);

        if(accepted)
            return relevance_->Filter(params);
        else
            return false;
    }
    else
        return true;
}
