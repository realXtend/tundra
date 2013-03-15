// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "A3Filter.h"
#include "RelevanceFilter.h"
#include "EuclideanDistanceFilter.h"
#include "InterestManager.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

A3Filter::A3Filter(InterestManager *im, int criticalrange, int maxrange, int updateinterval, bool enabled) :
    im_(im),
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

QString A3Filter::ToString()
{
    return QString("A3");
}

bool A3Filter::Filter(const IMParameters& params)
{
    if(enabled_)
    {
        if(euclideandistance_->Filter(params))
            return true;

        if(params.dot <= 0)
        {
            im_->UpdateRelevance(params.connection, params.changed_entity->Id(), 0);
            return false;
        }

        else if(relevance_->Filter(params))
            return true;

        else
            return false;
    }
    else
        return true;
}
