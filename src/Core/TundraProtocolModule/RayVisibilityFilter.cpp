// For conditions of distribution and use, see copyright notice in LICENSE

#include "map"

#include "Geometry/Ray.h"
#include "Entity.h"
#include "OgreWorld.h"
#include "Scene.h"
#include "InterestManager.h"
#include "RayVisibilityFilter.h"

RayVisibilityFilter::RayVisibilityFilter(InterestManager *im, int r, int interval, bool enabled) :
    im_(im),
    range_(r),
    raycastinterval_(interval),
    MessageFilter(RAY_VISIBILITY, enabled)
{

}

bool RayVisibilityFilter::Filter(IMParameters params)
{
    if(enabled_)
    {
        float cutoffrange = range_ * range_;

        if(params.headless)
            return true;

        if(params.distance < cutoffrange)  //If the entity is close enough, only then do a raycast
        {
            int lastRaycasted = 0;

            /*Check when was the last time we raycasted and dont do it if its not the time*/
            lastRaycasted = im_->FindLastRaycastedEntity(params.changed_entity->Id());

            int currentTime = im_->ElapsedTime();

            if(lastRaycasted + raycastinterval_ > currentTime)
            {
                std::map<entity_id_t, bool>::iterator it2;

                it2 = params.connection->syncState->visibleEntities.find(params.changed_entity->Id());

                if(it2 == params.connection->syncState->visibleEntities.end())
                    return false;

                if(it2->second == true) //bool which contains a value determining if the entity was visible to the user last time it was raycasted
                    return true;

                else
                    return false;
            }

            else
            {
                Ray ray(params.client_position, (params.entity_position - params.client_position).Normalized());

                RaycastResult *result = 0;
                OgreWorldPtr w = params.scene->GetWorld<OgreWorld>();

                result = w->Raycast(ray, 0xFFFFFFFF);

                im_->UpdateLastRaycastedEntity(params.changed_entity->Id());

                if(result && result->entity && result->entity->Id() == params.changed_entity->Id())  //If the ray hit someone and its our target entity
                {
                    im_->UpdateEntityVisibility(params.connection, params.changed_entity->Id(), true);
                    return true;
                }
                else
                {
                    im_->UpdateEntityVisibility(params.connection, params.changed_entity->Id(), false);
                    return false;
                }
            }
        }
        else
            return false;
    }
    else
        return true;
}
