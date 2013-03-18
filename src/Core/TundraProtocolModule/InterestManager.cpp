// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "Scene.h"
#include "SyncState.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "InterestManager.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

InterestManager* InterestManager::thisPointer_ = NULL;

InterestManager::InterestManager()
{
    activeFilter_ = 0;
    timer_ = new QTime();
    timer_->start();
}

InterestManager::~InterestManager()
{
    thisPointer_ = 0;
    delete timer_;
    delete activeFilter_;
    timer_ = 0;
    activeFilter_ = 0;
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
    PROFILE(Interest_Management);

    ScenePtr scene = scene_.lock();

    if (!scene)
        return true;

    EC_Placeable *entity_location = changed_entity->GetComponent<EC_Placeable>().get();

    if(!conn->syncState->locationInitialized || !entity_location) //If the client hasn't informed the server about the orientation yet, do not proceed
        return true;

    bool accepted = false;  //By default, we assume that the update will be rejected

    Quat client_orientation = conn->syncState->clientOrientation.Normalized();

    params_.client_position = conn->syncState->clientLocation;      //Client location vector
    params_.entity_position = entity_location->transform.Get().pos; //Entitys location vector

    float3 d = params_.client_position - params_.entity_position;
    float3 v = params_.entity_position - params_.client_position;   //Calculate the vector between the player and the changed entity by substracting their location vectors
    float3 f = client_orientation.Mul(scene->ForwardVector());      //Calculate the forward vector of the client

    params_.headless = headless;
    params_.dot = v.Dot(f);                                          //Finally the dot product is calculated so we know if the entity is in front of the player or not
    params_.distance = d.LengthSq();
    params_.scene = scene;
    params_.changed_entity = changed_entity;
    params_.connection = conn;
    params_.relAccepted = false;

    if(activeFilter_ != 0)
        accepted = activeFilter_->Filter(params_);

    if(accepted)
    {
        UpdateLastUpdatedEntity(params_.connection, params_.changed_entity->Id());
        return true;
    }
    else
        return false;
}

void InterestManager::UpdateRelevance(UserConnectionPtr conn, entity_id_t id, float relevance)
{
    std::map<entity_id_t, float>::iterator it;

    it = conn->syncState->relevanceFactors.find(id);

    if(it == conn->syncState->relevanceFactors.end()) //Theres no entry with this entity_id
        conn->syncState->relevanceFactors.insert(std::make_pair(id, relevance));
    else
        it->second = relevance;
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

void InterestManager::UpdateLastUpdatedEntity(UserConnectionPtr conn, entity_id_t id)
{
    std::map<entity_id_t, float>::iterator it;

    it = conn->syncState->lastUpdatedEntitys_.find(id);

    if(it == conn->syncState->lastUpdatedEntitys_.end())
        conn->syncState->lastUpdatedEntitys_.insert(std::make_pair(id, ElapsedTime()));
    else
        it->second = ElapsedTime();
}

float InterestManager::FindLastUpdatedEntity(UserConnectionPtr conn, entity_id_t id)
{
    std::map<entity_id_t, float>::iterator it;

    it = conn->syncState->lastUpdatedEntitys_.find(id);

    if(it == conn->syncState->lastUpdatedEntitys_.end())
        return 0;
    else
        return it->second;
}

void InterestManager::UpdateLastRaycastedEntity(UserConnectionPtr conn, entity_id_t id)
{
    std::map<entity_id_t, float>::iterator it;

    it = conn->syncState->lastRaycastedEntitys_.find(id);

    if(it == conn->syncState->lastRaycastedEntitys_.end())
        conn->syncState->lastRaycastedEntitys_.insert(std::make_pair(id, ElapsedTime()));
    else
        it->second = ElapsedTime();
}

float InterestManager::FindLastRaycastedEntity(UserConnectionPtr conn, entity_id_t id)
{
    std::map<entity_id_t, float>::iterator it;

    it = conn->syncState->lastRaycastedEntitys_.find(id);

    if(it == conn->syncState->lastRaycastedEntitys_.end())
        return 0;
    else
        return it->second;
}
