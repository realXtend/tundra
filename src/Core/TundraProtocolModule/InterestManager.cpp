// For conditions of distribution and use, see copyright notice in LICENSE

#include "Scene.h"
#include "SyncState.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "InterestManager.h"

InterestManager* InterestManager::thisPointer_ = NULL;

InterestManager::InterestManager()
{
    timer_ = new QTime();
    timer_->start();
}

InterestManager::~InterestManager()
{
    thisPointer_ = NULL;
    delete timer_;
    delete activeFilter_;
}

InterestManager* InterestManager::getInstance()
{
    if(!thisPointer_)
        thisPointer_ = new InterestManager;

    return thisPointer_;
}

void InterestManager::AssignFilter(MessageFilter *filter)
{
    activeFilter_ = filter;
}

int InterestManager::ElapsedTime()
{
    return timer_->elapsed();
}

bool InterestManager::CheckRelevance(UserConnectionPtr conn, Entity* changed_entity, SceneWeakPtr scene_, bool headless)
{
    bool accepted = true;
    IMParameters params;
    ScenePtr scene = scene_.lock();

    if (!scene)
        return true;

    EC_Placeable *entity_location = changed_entity->GetComponent<EC_Placeable>().get();

    if(!conn->syncState->clientLocation.IsFinite() || !entity_location) //If the client hasn't informed the server about the orientation yet, do not proceed
        return true;

    Quat client_orientation = conn->syncState->clientOrientation;

    params.client_position = conn->syncState->clientLocation;      //Client location vector
    params.entity_position = entity_location->transform.Get().pos; //Entitys location vector

    float3 d = params.client_position - params.entity_position;
    float3 v = params.entity_position - params.client_position;    //Calculate the vector between the player and the changed entity by substracting their location vectors
    float3 f = client_orientation.Mul(scene->ForwardVector());     //Calculate the forward vector of the client

    params.dot = v.Dot(f);                                         //Finally the dot product is calculated so we know if the entity is in front of the player or not
    params.distance = d.LengthSq();
    params.connection = conn;
    params.changed_entity = changed_entity;
    params.scene = scene;
    params.headless = headless;

    if(activeFilter_ != 0)
        accepted = activeFilter_->Filter(params);

    if(accepted)
        UpdateLastUpdatedEntity(changed_entity->Id());

    return accepted;
}

void InterestManager::UpdateEntityVisibility(UserConnectionPtr conn, entity_id_t id, bool visible)
{
    std::map<entity_id_t, bool>::iterator it;

    it = conn->syncState->visibleEntities.find(id);

    if(it == conn->syncState->visibleEntities.end()) //Theres no entry with this entity_id
        conn->syncState->visibleEntities.insert(std::make_pair(id, visible));
    else
        it->second = visible;
}

void InterestManager::UpdateLastUpdatedEntity(entity_id_t id)
{
    std::map<entity_id_t, int>::iterator it;

    it = lastUpdatedEntitys_.find(id);

    if(it == lastUpdatedEntitys_.end())
        lastUpdatedEntitys_.insert(std::make_pair(id, ElapsedTime()));
    else
        it->second = ElapsedTime();
}

int InterestManager::FindLastUpdatedEntity(entity_id_t id)
{
    std::map<entity_id_t, int>::iterator it;

    it = lastUpdatedEntitys_.find(id);

    if(it == lastUpdatedEntitys_.end())
        return 0;
    else
        return it->second;
}

void InterestManager::UpdateLastRaycastedEntity(entity_id_t id)
{
    std::map<entity_id_t, int>::iterator it;

    it = lastRaycastedEntitys_.find(id);

    if(it == lastRaycastedEntitys_.end())
        lastRaycastedEntitys_.insert(std::make_pair(id, ElapsedTime()));
    else
        it->second = ElapsedTime();
}

int InterestManager::FindLastRaycastedEntity(entity_id_t id)
{
    std::map<entity_id_t, int>::iterator it;

    it = lastRaycastedEntitys_.find(id);

    if(it == lastRaycastedEntitys_.end())
        return 0;
    else
        return it->second;
}
