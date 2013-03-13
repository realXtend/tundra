// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "Entity.h"
#include "InterestManager.h"
#include "RelevanceFilter.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

RelevanceFilter::RelevanceFilter(InterestManager *im, int r, int cr, int interval, bool enabled) :
    im_(im),
    range_(r),
    critical_range_(cr),
    updateinterval_(interval),
    MessageFilter(RELEVANCE, enabled)
{

}

QString RelevanceFilter::ToString()
{
    return QString("Relevance");
}

bool RelevanceFilter::Filter(const IMParameters& params)
{   
    if(enabled_)
    {
        float relevancefactor = 0;

        relevancefactor = 1.f - (params.distance - (critical_range_ * critical_range_)) / ((range_ * range_) - (critical_range_ * critical_range_));

        if(relevancefactor <= 0)
            relevancefactor = 0;

        im_->UpdateRelevance(params.connection, params.changed_entity->Id(), relevancefactor);

        if(relevancefactor == 0)
            return false;

        else
        {
            float currentTime = im_->ElapsedTime();
            float lastUpdated = im_->FindLastUpdatedEntity(params.connection, params.changed_entity->Id());

            if(lastUpdated + (updateinterval_ * (1.f - relevancefactor)) < currentTime)
                return true;
            else
                return false;
        }
    }
    else
        return true;
}
