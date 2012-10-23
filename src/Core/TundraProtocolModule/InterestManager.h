// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QTime>

#include "MessageFilter.h"
#include "A3Filter.h"
#include "EA3Filter.h"
#include "EuclideanDistanceFilter.h"
#include "RayVisibilityFilter.h"
#include "RelevanceFilter.h"

class InterestManager
{

public:

    /// Destructor, sets instanceFlag to false so getInstance creates new on request
    ~InterestManager();

    /// Returns singleton pointer
    static InterestManager* getInstance();

    /// Assign a filter to the IM
    void AssignFilter(MessageFilter *filter);

    /// Main entrance method for the filtering process
    bool CheckRelevance(UserConnectionPtr userconnection, Entity* changed_entity, SceneWeakPtr scene, bool headless);

    /// Returns the current active filtering time in milliseconds
    int ElapsedTime();

    /// Updates a specific map which contains a list of entities and their visibilities. (client specific map)
    void UpdateEntityVisibility(UserConnectionPtr connection, entity_id_t id, bool visible);

    /// Updates a map that contains the timestamps that describe when a specific entity was last updated
    void UpdateLastUpdatedEntity(entity_id_t id);

    /// Updates a map that contains the timestamps that describe when a specific entity was last raycasted
    void UpdateLastRaycastedEntity(entity_id_t id);

    /// Utility method for checking when a specific entity has been updated
    int FindLastUpdatedEntity(entity_id_t id);

    /// Utility method for checking when a specific entity has been raycasted
    int FindLastRaycastedEntity(entity_id_t id);

private:

    /// Private constructor because singleton instance is gotten from getInstance()
    InterestManager();

    /// Pointer to this singleton class
    static InterestManager* thisPointer_;

    /// Timer handling the update intervals
    QTime *timer_;

    /// The filter that is used inside the IM
    MessageFilter* activeFilter_;

    /// Couple of maps containing the timestamps of last updates and raycasts
    std::map<entity_id_t, int> lastUpdatedEntitys_;
    std::map<entity_id_t, int> lastRaycastedEntitys_;
};
