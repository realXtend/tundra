// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include "Math/float3.h"

#include <QObject>

class Scene;
class Entity;
class EC_Camera;

/// Result of a raycast to the rendered representation of a scene.
/** Other fields are valid only if entity is non-null
    @sa OgreWorld
    @todo Remove the QObject inheritance here, and expose as a struct to scripts. */
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
    /// Triangle index in submesh
    unsigned index;
    /// U coord in entity. 0 if no texture mapping
    float u;
    /// V coord in entity. 0 if no texture mapping
    float v;
};

/// Describes the system renderer.
/** @note This class is not an abstract reimplementable interface, but exists only internally for DLL dependency inversion
        purposes between Framework and OgreRenderingModule. This interface is only internal to Framework. Do not extend this 
        interface. Avoid using it in client code, and prefer directly getting the Renderer object from OgreRenderingModule. */
class IRenderer
{
public:
    IRenderer() {}

    virtual ~IRenderer() {}

    /// Renders the scene.
    /// Do not call this function yourself. It is only used internally by Framework to drive Ogre rendering.
    virtual void Render(float frameTime) = 0;

    /// Returns the Entity which contains the currently active camera that is used to render on the main window.
    /// The returned Entity is guaranteed to have an EC_Camera component, and it is guaranteed to be attached to a scene.
    virtual Entity *MainCamera() = 0;

    /// Returns the EC_Camera of the main camera, or 0 if no main camera is active.
    virtual EC_Camera *MainCameraComponent() = 0;

    /// Returns the Scene the current active main camera is in, or 0 if no main camera is active.
    virtual Scene *MainCameraScene() = 0;

    /// Returns an unique string name for a new object. This function is intended to be used to generate unique names for Ogre resources.
    virtual std::string GetUniqueObjectName(const std::string &prefix) = 0;

    /// \deprecated Do not use this function. Instead use OgreWorld::Raycast or Physics::PhysicsWorld::Raycast. (In script code scene.ogre.Raycast or scene.physics.Raycast)
    /// \todo This will be removed as soon as SceneInteract is out of core.
    virtual RaycastResult *Raycast(int x, int y) = 0;
};
