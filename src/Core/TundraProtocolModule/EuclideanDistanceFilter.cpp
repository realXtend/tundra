// For conditions of distribution and use, see copyright notice in LICENSE

#include "InterestManager.h"
#include "EuclideanDistanceFilter.h"
#include "Profiler.h"

EuclideanDistanceFilter::EuclideanDistanceFilter(InterestManager *im, int criticaldistance, bool enabled) :
    im_(im),
    radius_(criticaldistance),
    MessageFilter(EUCLIDEAN_DISTANCE, enabled)
{

}

bool EuclideanDistanceFilter::Filter(IMParameters params)
{
    PROFILE(EuclideanDistance_Filter);

    if(enabled_)
    {
        float cutoffrange = radius_ * radius_;

        if(params.distance < cutoffrange) //If the entity is inside the critical area then always allow updates
            return true;
        else
            return false;
    }
    else
        return true;
}
