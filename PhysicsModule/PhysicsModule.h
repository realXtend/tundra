// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsModule_h
#define incl_Physics_PhysicsModule_h

#include "PhysicsModuleApi.h"
#include "IModule.h"
#include "SceneFwd.h"

#include <LinearMath/btIDebugDraw.h>

#include <set>
#include <QObject>

namespace Ogre
{
    class Mesh;
}

class btVector3;
class btTriangleMesh;
class QScriptEngine;
class EC_RigidBody;
struct ConsoleCommandResult;

namespace Physics
{

struct ConvexHullSet;
class PhysicsWorld;
class DebugLines;

class PHYSICS_MODULE_API PhysicsModule : public IModule, public btIDebugDraw
{
    Q_OBJECT

    friend class EC_RigidBody;
    
public:
    PhysicsModule();
    virtual ~PhysicsModule();

    void Load();
    void Initialize();
    void PostInitialize();
    void Update(f64 frametime);
    void Uninitialize();
        
    /// IDebugDraw override
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    
    /// IDebugDraw override
    virtual void reportErrorWarning(const char* warningString);
    
    /// IDebugDraw override
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}
    
    /// IDebugDraw override
    virtual void draw3dText(const btVector3& location,const char* textString) {}
    
    /// IDebugDraw override
    virtual void setDebugMode(int debugMode) { debugDrawMode_ = debugMode; }
    
    /// IDebugDraw override
    virtual int getDebugMode() const { return debugDrawMode_; }
    
    /// Get a Bullet triangle mesh corresponding to an Ogre mesh.
    /** If already has been generated, returns the previously created one
     */
    boost::shared_ptr<btTriangleMesh> GetTriangleMeshFromOgreMesh(Ogre::Mesh* mesh);

    /// Get a Bullet convex hull set (using minimum recursion, not very accurate but fast) corresponding to an Ogre mesh.
    /** If already has been generated, returns the previously created one
     */
    boost::shared_ptr<ConvexHullSet> GetConvexHullSetFromOgreMesh(Ogre::Mesh* mesh);
    
    /// Create a physics world for a scene
    /** \param scene Scene into which to create
        \param isClient If true, physics will be only simulated for local entities
     */
    Physics::PhysicsWorld* CreatePhysicsWorldForScene(ScenePtr scene, bool isClient);
    
    /// Return the physics world for a scene if it exists
    Physics::PhysicsWorld* GetPhysicsWorldForScene(ScenePtr scene);
    
    /// Return the physics world for a scene if it exists
    Physics::PhysicsWorld* GetPhysicsWorldForScene(SceneManager* sceneraw);
    
public slots:

    /// Toggles physics debug geometry
    void ConsoleToggleDebugGeometry();

    /// Stops physics
    void ConsoleStopPhysics();
    
    /// Starts physics
    void ConsoleStartPhysics();
    
    /// Autoassigns static rigid bodies with collision meshes to visible meshes
    void ConsoleAutoCollisionMesh();

    /// Get a physics world for a scene. This version meant for scripts
    /** Note: the parameter is a QObject*, because typically the scriptengine's dynamic property scene is used to query for physicsobject.
        But it seems to lose its knowledge of actually being a SceneManager*, and returns null. This version will dynamic cast
        to SceneManager*
     */ 
    Physics::PhysicsWorld* GetPhysicsWorld(QObject* scene);
    
    /// Enable/disable physics simulation
    void SetRunPhysics(bool enable);
    
    /// Enable/disable debug geometry
    void SetDrawDebugGeometry(bool enable);
    
    /// Get debug geometry enabled status
    bool GetDrawDebugGeometry() const { return drawDebugGeometry_; }
    
    /// Initialize physics datatypes for a script engine
    void OnScriptEngineCreated(QScriptEngine* engine);
    
private slots:
    /// Scene has been removed, so delete also the physics world (if exists)
    void OnSceneRemoved(SceneManager* scene);
    
private:
    /// Update debug geometry manual object, if physics debug drawing is on
    void UpdateDebugGeometry();
    
    typedef std::map<SceneManager*, boost::shared_ptr<Physics::PhysicsWorld> > PhysicsWorldMap;
    /// Map of physics worlds assigned to scenes
    PhysicsWorldMap physicsWorlds_;
    
    typedef std::map<std::string, boost::shared_ptr<btTriangleMesh> > TriangleMeshMap;
    /// Bullet triangle meshes generated from Ogre meshes
    TriangleMeshMap triangleMeshes_;

    typedef std::map<std::string, boost::shared_ptr<ConvexHullSet> > ConvexHullSetMap;
    /// Bullet convex hull sets generated from Ogre meshes
    ConvexHullSetMap convexHullSets_;
    
    /// Debug geometry enabled flag
    bool drawDebugGeometry_;
    
    /// Debug geometry manually enabled/disabled (with physicsdebug console command). If true, do not automatically enable/disable debug geometry anymore
    bool drawDebugManuallySet_;
    
    /// Lines object for the debug geometry
    DebugLines* debugGeometryObject_;
    
    /// Whether should run physics. Default true
    bool runPhysics_;
    
    /// Bullet debug draw / debug behaviour flags
    int debugDrawMode_;
    
    /// Debug draw-enabled rigidbodies. Note: these pointers are never dereferenced, it is just used for counting
    std::set<EC_RigidBody*> debugRigidBodies_;
};

}

#endif
