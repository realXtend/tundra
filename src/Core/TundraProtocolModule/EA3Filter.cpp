// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "InterestManager.h"
#include "EA3Filter.h"
#include "EuclideanDistanceFilter.h"
#include "RayVisibilityFilter.h"
#include "RelevanceFilter.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

EA3Filter::EA3Filter(InterestManager *im, int criticalrange, int maxrange, int raycastinterval, int updateinterval, bool enabled) :
    im_(im),
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

QString EA3Filter::ToString()
{
    return QString("EA3");
}

bool EA3Filter::Filter(const IMParameters& params)
{  
    if(enabled_)
    {
        if(euclideandistance_->Filter(params))
            return true;

        if(params.dot <= 0) //If the entity is behind the client
        {
            im_->UpdateEntityVisibility(params.connection, params.changed_entity->Id(), false);
            im_->UpdateRelevance(params.connection, params.changed_entity->Id(), 0);
            return false;
        }

        else if(rayvisibility_->Filter(params)) //If rayvisibility accepts it
            return relevance_->Filter(params);  //Pass it through the relevance filter and return

        else
            return false;
    }
    else
        return true;
}
