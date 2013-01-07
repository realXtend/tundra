// For conditions of distribution and use, see copyright notice in LICENSE

#include "map"

#include "Geometry/Ray.h"
#include "EC_Camera.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "OgreWorld.h"
#include "Scene.h"
#include "InterestManager.h"
#include "RayVisibilityFilter.h"
#include "LoggingFunctions.h"

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
            std::map<entity_id_t, bool>::iterator it;

            it = params.connection->syncState->visibleEntities.find(params.changed_entity->Id());

            /*Check when was the last time we raycasted and dont do it if its not the time*/
            int lastRaycasted = im_->FindLastRaycastedEntity(params.connection, params.changed_entity->Id());
            int currentTime = im_->ElapsedTime();

            if(it != params.connection->syncState->visibleEntities.end() && (lastRaycasted + raycastinterval_) > currentTime)   //If the entity is located from the map
            {
                if(it->second == true) //bool which contains a value determining if the entity was visible to the user last time it was raycasted
                {
#ifdef IM_DEBUG
                    if(params.connection->ConnectionId() == 1)
                        ::LogInfo("Not the time to raycast, returning true. Last " + QString::number(lastRaycasted + raycastinterval_) + " Current " + QString::number(currentTime));
#endif
                    return true;
                }
                else
                {
#ifdef IM_DEBUG
                    if(params.connection->ConnectionId() == 1)
                        ::LogInfo("Not the time to raycast, returning false. Last " + QString::number(lastRaycasted + raycastinterval_) + " Current " + QString::number(currentTime));
#endif
                    return false;
                }
            }

            else
            {
//#ifdef IM_DEBUG
                Ray ray(params.client_position, (params.entity_position - params.client_position).Normalized());
                RaycastResult *result = 0;
                OgreWorldPtr w = params.scene->GetWorld<OgreWorld>();

                result = w->Raycast(ray, 0xFFFFFFFF);
//#endif

#if 0
                /// Hack, lets try to detect entities with EC_Cameras IsEntityVisible function
                EntityPtr cameraentity = params.scene->CreateEntity(0, QStringList(), AttributeChange::LocalOnly, false, false);

                ComponentPtr cameracomponent = cameraentity->GetOrCreateComponent("EC_Camera");
                EC_Camera *camera = checked_static_cast<EC_Camera*>(cameracomponent.get());

                ComponentPtr placeablecomponent = cameraentity->GetOrCreateComponent("EC_Placeable");
                EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeablecomponent.get());

                placeable->SetPosition(params.connection->syncState->clientLocation);
                placeable->SetOrientation(params.connection->syncState->clientOrientation.Normalized());

                // Crank up the FOV
                camera->setverticalFov(179);

                bool visible = camera->IsEntityVisible(params.changed_entity);

                params.scene->DeleteEntityById(cameraentity->Id());
#endif
                im_->UpdateLastRaycastedEntity(params.connection, params.changed_entity->Id());

                if(result && result->entity && result->entity->Id() == params.changed_entity->Id())  //If the ray hit someone and its our target entity
                {
                    im_->UpdateEntityVisibility(params.connection, params.changed_entity->Id(), true);
#ifdef IM_DEBUG
                    ::LogInfo("Entity " + QString::number(params.changed_entity->Id()) + " is visible to connection " + QString::number(params.connection->ConnectionId()));
#endif
                    return true;
                }
                else
                {
                    im_->UpdateEntityVisibility(params.connection, params.changed_entity->Id(), false);
                    im_->UpdateRelevance(params.connection, params.changed_entity->Id(), 0);
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
