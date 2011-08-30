// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "Math/float3.h"
#include "Math/Quat.h"

#include "CoreTypes.h"

#include <QObject>
#include <QVariant>
#include <set>

class QRect;

class Entity;

/// Result of a raycast. Other fields are valid only if entity_ is non-null
///\todo Remove the QObject inheritance here, and expose as a struct to scripts.
class RaycastResult : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(Entity* entity READ getentity);
    Entity* getentity() const { return entity; }
    Q_PROPERTY(float3 pos READ getpos);
    float3 getpos() const { return pos; }
    Q_PROPERTY(float3 normal READ getnormal);
    float3 getnormal() const { return normal; }
    Q_PROPERTY(unsigned submesh READ getsubmesh);
    unsigned getsubmesh() const { return submesh; }
    Q_PROPERTY(unsigned index READ getindex);
    unsigned getindex() const { return index; }
    Q_PROPERTY(float u READ getu);
    float getu() const { return u; }
    Q_PROPERTY(float v READ getv);
    float getv() const { return v; }

    /// Entity that was hit, null if none
    Entity* entity;
    /// World coordinates of hit position
    float3 pos;
    /// World face normal of hit
    float3 normal;
    /// Submesh index in entity, starting from 0
    unsigned submesh;
    /// Triangle index in mesh
    unsigned index;
    /// U coord in entity. 0 if no texture mapping
    float u;
    /// V coord in entity. 0 if no texture mapping
    float v;
};

/// Describes the system renderer.
/// \note This class is not an abstract reimplementable interface, but exists only internally for DLL dependency inversion
///    purposes between Framework and OgreRenderingModule. This interface is only internal to Framework. Do not extend this 
///    interface or use it in client code.
class IRenderer
{
public:
    IRenderer() {}

    virtual ~IRenderer() {}

    /// Renders the scene.
    virtual void Render(float frameTime) = 0;
    
    /// \deprecated Do not use this function. Instead use OgreWorld::Raycast or PhysicsWorld::Raycast. (In script code scene.ogre.Raycast or scene.physics.Raycast)
    virtual RaycastResult* Raycast(int x, int y) = 0;

    /// \deprecated Do not use this function. Instead use OgreWorld::Raycast or PhysicsWorld::Raycast. (In script code scene.ogre.Raycast or scene.physics.Raycast)
    virtual RaycastResult* Raycast(int x, int y, unsigned layerMask) = 0;
    
    /// \deprecated Do not use this function. Instead use OgreWorld::FrustumQuery.
    virtual QList<Entity*> FrustumQuery(QRect &viewrect) = 0;

    /// \deprecated Do not use this function.
    /// \todo Reimplement in EC_Camera.
    virtual std::string GetUniqueObjectName(const std::string &prefix) = 0;
};
