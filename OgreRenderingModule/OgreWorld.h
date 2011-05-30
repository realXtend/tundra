// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreDefines.h"
#include "IRenderer.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "SceneFwd.h"

#include <QObject>
#include <QList>

#include <OgreRenderQueue.h>

#include <boost/enable_shared_from_this.hpp>

class EC_Camera;
class Framework;

namespace OgreRenderer
{
    class GaussianListener;
    class Renderer;
}

/// Contains the Ogre representation of a scene, ie. the Ogre Scene
class OGRE_MODULE_API OgreWorld : public QObject, public boost::enable_shared_from_this<OgreWorld>
{
    Q_OBJECT
    
public:
    /// Construct. Is called by the OgreRenderingModule upon the creation of a new scene
    OgreWorld(OgreRenderer::Renderer* renderer, ScenePtr scene);
    /// Destruct. Fully destroy the Ogre scene
    virtual ~OgreWorld();
    
    /// Dynamic scene property name
    static const char* PropertyNameStatic()
    {
        return "ogre";
    }
    
    /// Returns an unique name to create Ogre objects that require a mandatory name. Calls the parent Renderer
    /** @param prefix Prefix for the name. */
    std::string GetUniqueObjectName(const std::string &prefix);
    
public slots:
    /// Do raycast into the world from viewport coordinates, using all selection layers
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** The coordinates are a position in the render window, not scaled to [0,1].
        \param x Horizontal position for the origin of the ray
        \param y Vertical position for the origin of the ray
        \return Raycast result structure
    */
    RaycastResult* Raycast(int x, int y);

    /// Do raycast into the world from viewport coordinates, using specific selection layer(s)
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** The coordinates are a position in the render window, not scaled to [0,1].
        \param x Horizontal position for the origin of the ray
        \param y Vertical position for the origin of the ray
        \param layerMask Which selection layer(s) to use (bitmask)
        \return Raycast result structure
    */
    RaycastResult* Raycast(int x, int y, unsigned layerMask);
    
    /// Do a frustum query to the world from viewport coordinates.
    /// \todo This function will be removed and replaced with a function Scene::Intersect.
    /** Returns the found entities as a QVariantList so that
        Python and Javascript can get the result directly from here.
        \param viewrect The query rectangle in 2d window coords.
    */
    QList<Entity*> FrustumQuery(QRect &viewrect);
    
    /// Return whether a single entity is visible in the currently active camera
    bool IsEntityVisible(Entity* entity) const;
    
    /// Get visible entities in the currently active camera
    QList<Entity*> GetVisibleEntities() const;
    
    /// Return whether the currently active camera is in this scene
    bool IsActive() const;
    
    /// Start tracking an entity's visibility within this scene, using any camera(s)
    /** After this, connect either to the EntityEnterView and EntityLeaveView signals, or the entity's EnterView & LeaveView signals,
       to be notified of the visibility change(s)
     */
    void StartViewTracking(Entity* entity);
    
    /// Stop tracking an entity's visibility
    void StopViewTracking(Entity* entity);
    
    /// Return the Renderer instance
    OgreRenderer::Renderer* GetRenderer() const { return renderer_; }
    /// Return the Ogre scene manager
    Ogre::SceneManager* GetSceneManager() { return sceneManager_; }
    /// Return the parent scene
    ScenePtr GetScene() { return scene_.lock(); }
    
signals:
    /// An entity has entered the view
    void EntityEnterView(Entity* entity);
    
    /// An entity has left the view
    void EntityLeaveView(Entity* entity);
    
private slots:
    /// Handle frame update. Used for entity visibility tracking
    void OnUpdated(float timeStep);
    
private:
    /// Setup shadows
    void SetupShadows();
    
    /// Return the currently active camera component, if it belongs to this scene. Else return null
    EC_Camera* VerifyCurrentSceneCameraComponent() const;
    
    /// Verify that the currently active camera belongs to this scene. Return its OgreCamera, or null if mismatch
    Ogre::Camera* VerifyCurrentSceneCamera() const;
    
    /// Framework
    Framework* framework_;
    
    /// Parent renderer
    OgreRenderer::Renderer* renderer_;
    
    /// Parent scene
    SceneWeakPtr scene_;
    
    /// Ogre scenemanager
    Ogre::SceneManager* sceneManager_;
    
    /// Ray for raycasting, reusable
    Ogre::RaySceneQuery *rayQuery_;
    
    /// Ray query result
    RaycastResult result_;
    
    /// Soft shadow gaussian listeners
    std::list<OgreRenderer::GaussianListener *> gaussianListeners_;
    
    /// Visible entity ID's during this frame. Acquired from the active camera. Not updated if no entities are tracked for visibility.
    std::set<entity_id_t> visibleEntities_;
    
    /// Visible entity ID's during last frame. Acquired from the active camera. Not updated if no entities are tracked for visibility.
    std::set<entity_id_t> lastVisibleEntities_;
    
    /// Entities being tracked for visibility changes
    std::vector<EntityWeakPtr> visibilityTrackedEntities_;
};

