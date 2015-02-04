// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleFwd.h"
#include "Math/float3.h"
#include "SceneFwd.h"

struct IMParameters
{
    /*Currently implemented filters*/
    enum IMFilter
    {
        A3,
        EA3,
        EUCLIDEAN_DISTANCE,
        RAY_VISIBILITY,
        RELEVANCE
    };

    IMParameters() :    headless(false),
                        dot(0),
                        distance(0),
                        client_position(float3::nan),
                        entity_position(float3::nan),
                        relAccepted(false) {}

    bool headless;                  //Is the server running in headless mode or not
    float dot;                      //Dot product between the client and the entity (is the entity is in front of the camera or not)
    float distance;                 //Distance between the client and the entity
    float3 client_position;
    float3 entity_position;
    ScenePtr scene;
    Entity* changed_entity;
    UserConnectionPtr connection;   //Client connection
    bool relAccepted;
};

class MessageFilter
{
public:

    MessageFilter(IMParameters::IMFilter type, bool enabled) : type_(type), enabled_(enabled) {}
    virtual ~MessageFilter() {}

    virtual bool Filter(const IMParameters& params) = 0;

    virtual void SetEnabled(bool e)     { enabled_ = e; }
    virtual bool Enabled()              { return enabled_; }
    virtual IMParameters::IMFilter Info()             { return type_; }
    virtual QString ToString() = 0;

protected:
    IMParameters::IMFilter type_;
    bool enabled_;
};
