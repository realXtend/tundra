// For conditions of distribution and use, see copyright notice in LICENSE

#include "A3Filter.h"
#include "RelevanceFilter.h"
#include "EuclideanDistanceFilter.h"
#include "InterestManager.h"

A3Filter::A3Filter(InterestManager *im, int criticalrange, int maxrange, int updateinterval, bool enabled) :
    MessageFilter(A3, enabled)
{
    euclideandistance_ = new EuclideanDistanceFilter(im, criticalrange, true);
    relevance_ = new RelevanceFilter(im, maxrange, criticalrange, updateinterval, true);
}

A3Filter::~A3Filter()
{
    delete euclideandistance_;
    delete relevance_;
}

bool A3Filter::Filter(IMParameters params)
{
    bool accepted = true;

    if(enabled_)
    {
        accepted = euclideandistance_->Filter(params);

        if(accepted)
            return true;

        else if(params.dot >= 0)
            return relevance_->Filter(params);

        else
            return false;
    }
    else
        return true;
}
