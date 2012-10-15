// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SyncState.h"
#include "EC_Placeable.h"
#include "UserConnection.h"

/*Currently implemented filters*/
enum IMFilter
{
    A3,
    EA3,
    EUCLIDEAN_DISTANCE,
    RAY_VISIBILITY,
    RELEVANCE
};

struct IMParameters
{
    IMParameters() {}

    bool headless;                  //Is the server running in headless mode or not
    float dot;                      //Dot product between the client and the entity (is the entity is in front of the camera or not)
    float distance;                 //Distance between the client and the entity
    float3 client_position;
    float3 entity_position;
    ScenePtr scene;
    Entity* changed_entity;
    UserConnectionPtr connection;   //Client connection
};

class MessageFilter
{
public:

    MessageFilter(IMFilter type, bool enabled) : type_(type), enabled_(enabled) {}

    virtual bool Filter(IMParameters params) = 0;

    virtual void SetEnabled(bool e)     { enabled_ = e; }
    virtual bool Enabled()              { return enabled_; }
    virtual IMFilter Info()             { return type_; }

protected:
    IMFilter type_;
    bool enabled_;
};
