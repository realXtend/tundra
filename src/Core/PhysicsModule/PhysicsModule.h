// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "PhysicsModuleApi.h"
#include "PhysicsModuleFwd.h"
#include "IModule.h"
#include "SceneFwd.h"

#include <set>
#include <QObject>

namespace Ogre
{
    class Mesh;
}

class QScriptEngine;

#ifdef PROFILING
class QTreeWidgetItem;
#endif

namespace Physics
{
/// Provides physics rendering by utilizing Bullet.
class PHYSICS_MODULE_API PhysicsModule : public IModule
{
    Q_OBJECT
    Q_PROPERTY(float defaultPhysicsUpdatePeriod READ DefaultPhysicsUpdatePeriod WRITE SetDefaultPhysicsUpdatePeriod)
    Q_PROPERTY(int defaultMaxSubSteps READ DefaultMaxSubSteps WRITE SetDefaultMaxSubSteps)

public:
    PhysicsModule();
    virtual ~PhysicsModule();

    void Load();
    void Initialize();
    void Update(f64 frametime);
    void Uninitialize();
   
    /// Get a Bullet triangle mesh corresponding to an Ogre mesh.
    /** If already has been generated, returns the previously created one */
    shared_ptr<btTriangleMesh> GetTriangleMeshFromOgreMesh(Ogre::Mesh* mesh);

    /// Get a Bullet convex hull set (using minimum recursion, not very accurate but fast) corresponding to an Ogre mesh.
    /** If already has been generated, returns the previously created one */
    shared_ptr<ConvexHullSet> GetConvexHullSetFromOgreMesh(Ogre::Mesh* mesh);

    /// Set default physics update rate for new physics worlds
    void SetDefaultPhysicsUpdatePeriod(float updatePeriod);

    /// Return default physics update rate for new physics worlds
    float DefaultPhysicsUpdatePeriod() const { return defaultPhysicsUpdatePeriod_; }

    /// Set default physics max substeps for new physics worlds
    void SetDefaultMaxSubSteps(int steps);

    /// Return default physics max substeps for new physics worlds
    int DefaultMaxSubSteps() const { return defaultMaxSubSteps_; }

public slots:
    /// Toggles physics debug geometry
    void ToggleDebugGeometry();

    /// Stops physics for all physics worlds
    void StopPhysics();

    /// Starts physics for all physics worlds
    void StartPhysics();

    /// Autoassigns static rigid bodies with collision meshes to visible meshes
    void AutoCollisionMesh();

    /// Enable/disable physics simulation from all physics worlds
    void SetRunPhysics(bool enable);
    
    /// Initialize physics datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);

private slots:
    /// New scene has been created
    void OnSceneAdded(const QString &name);
    /// Scene is about to be removed
    void OnSceneRemoved(const QString &name);

private:
    typedef std::map<Scene*, shared_ptr<Physics::PhysicsWorld> > PhysicsWorldMap;
    /// Map of physics worlds assigned to scenes
    PhysicsWorldMap physicsWorlds_;
    
    typedef std::map<std::string, shared_ptr<btTriangleMesh> > TriangleMeshMap;
    /// Bullet triangle meshes generated from Ogre meshes
    TriangleMeshMap triangleMeshes_;

    typedef std::map<std::string, shared_ptr<ConvexHullSet> > ConvexHullSetMap;
    /// Bullet convex hull sets generated from Ogre meshes
    ConvexHullSetMap convexHullSets_;
    
    float defaultPhysicsUpdatePeriod_;
    int defaultMaxSubSteps_;
};

#ifdef PROFILING
void PHYSICS_MODULE_API UpdateBulletProfilingData(QTreeWidgetItem *treeRoot, int numFrames);
#endif

}

