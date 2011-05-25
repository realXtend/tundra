// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "Vector3D.h"
#include "Quaternion.h"

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
    Q_PROPERTY(Vector3df pos READ getpos);
    Vector3df getpos() const { return pos; }
    Q_PROPERTY(unsigned submesh READ getsubmesh);
    unsigned getsubmesh() const { return submesh; }
    Q_PROPERTY(float u READ getu);
    float getu() const { return u; }
    Q_PROPERTY(float v READ getv);
    float getv() const { return v; }

    /// Entity that was hit, null if none
    Entity* entity;
    /// World coordinates of hit position
    Vector3df pos;
    /// Submesh index in entity, starting from 0
    unsigned submesh;
    /// U coord in entity. 0 if no texture mapping
    float u;
    /// V coord in entity. 0 if no texture mapping
    float v;
};

/// Describes the system renderer.
class IRenderer
{
public:
    IRenderer() {}

    virtual ~IRenderer() {}

    /// Renders the scene
    virtual void Render() = 0;
    
    /// Do raycast into the currently active world from viewport coordinates, using all selection layers
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** The coordinates are a position in the render window, not scaled to [0,1].
        \param x Horizontal position for the origin of the ray
        \param y Vertical position for the origin of the ray
        \return Raycast result structure
    */
    virtual RaycastResult* Raycast(int x, int y) = 0;

    /// Do raycast into the currently active world from viewport coordinates, using specific selection layer(s)
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** The coordinates are a position in the render window, not scaled to [0,1].
        \param x Horizontal position for the origin of the ray
        \param y Vertical position for the origin of the ray
        \param layerMask Which selection layer(s) to use (bitmask)
        \return Raycast result structure
    */
    virtual RaycastResult* Raycast(int x, int y, unsigned layerMask) = 0;
    
    /// Do a frustum query to the currently active world from viewport coordinates.
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** Returns the found entities as a QVariantList so that
        Python and Javascript can get the result directly from here.
        \param viewrect The query rectangle in 2d window coords.
    */
    virtual QList<Entity*> FrustumQuery(QRect &viewrect) = 0;
    
    
    /// Returns render window width, or 0 if no window is opened
    /// \todo This function will be removed.
    virtual int GetWindowWidth() const = 0;

    /// Returns render window height, or 0 if no window is opened
    /// \todo This function will be removed.
    virtual int GetWindowHeight() const = 0;

    /// set maximum view distance
    virtual void SetViewDistance(float distance) = 0;

    /// get maximum view distance
    virtual float GetViewDistance() const = 0;

    /// get visible entities last frame
    virtual const std::set<entity_id_t>& GetVisibleEntities() = 0;
    
    virtual std::string GetUniqueObjectName(const std::string &prefix) = 0;
};
