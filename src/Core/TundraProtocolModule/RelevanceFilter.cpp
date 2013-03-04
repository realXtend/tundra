// For conditions of distribution and use, see copyright notice in LICENSE

#include "Entity.h"
#include "InterestManager.h"
#include "RelevanceFilter.h"

RelevanceFilter::RelevanceFilter(InterestManager *im, int r, int cr, int interval, bool enabled) :
    im_(im),
    range_(r),
    critical_range_(cr),
    updateinterval_(interval),
    MessageFilter(RELEVANCE, enabled)
{

}

bool RelevanceFilter::Filter(IMParameters params)
{
    if(enabled_)
    {
        float relevancefactor;

        relevancefactor = 1.f - (params.distance - (critical_range_ * critical_range_)) / ((range_ * range_) - (critical_range_ * critical_range_));

        im_->UpdateRelevance(params.connection, params.changed_entity->Id(), relevancefactor);

        if(relevancefactor <= 0)
            return false;

        else if(relevancefactor < 1.f)
        {
            int lastUpdated = im_->FindLastUpdatedEntity(params.changed_entity->Id());
            int currentTime = im_->ElapsedTime();

            if(lastUpdated + (updateinterval_ * (1 - relevancefactor)) > currentTime)
                return false;
        }

        return true;
    }
    else
        return true;
}
