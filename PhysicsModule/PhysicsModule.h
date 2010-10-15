// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsModule_h
#define incl_Physics_PhysicsModule_h

#include "PhysicsModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "RexTypes.h"
#include "SceneManager.h"
#include "LinearMath/btIDebugDraw.h"

namespace Ogre
{
    class ManualObject;
    class Mesh;
}

#include <QObject>

class btVector3;
class btTriangleMesh;

namespace Physics
{

class PhysicsWorld;

class PHYSICS_MODULE_API PhysicsModule : public QObject, public IModule, public btIDebugDraw
{
    Q_OBJECT

public:
    PhysicsModule();
    virtual ~PhysicsModule();

    MODULE_LOGGING_FUNCTIONS

    //! IModule override.
    void Load();
    
    //! IModule override.
    void PostInitialize();

    //! IModule override.
    void Update(f64 frametime);

    //! Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return moduleName; }

    //! Name of this module.
    static const std::string moduleName;
    
    //! Toggles physics debug geometry
    Console::CommandResult ConsoleToggleDebugGeometry(const StringVector& params);

    //! Stops physics
    Console::CommandResult ConsoleStopPhysics(const StringVector& params);
    
    //! Starts physics
    Console::CommandResult ConsoleStartPhysics(const StringVector& params);
    
    //! IDebugDraw override
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    
    //! IDebugDraw override
    virtual void reportErrorWarning(const char* warningString);
    
    //! IDebugDraw override
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}
    
    //! IDebugDraw override
    virtual void draw3dText(const btVector3& location,const char* textString) {}
    
    //! IDebugDraw override
    virtual void setDebugMode(int debugMode) { debugDrawMode_ = debugMode; }
    
    //! IDebugDraw override
    virtual int getDebugMode() const { return debugDrawMode_; }
    
    //! Get a Bullet triangle mesh corresponding to an Ogre mesh.
    /*! If already has been generated, returns the previously created one
     */
    boost::shared_ptr<btTriangleMesh> GetTriangleMeshFromOgreMesh(Ogre::Mesh* ogreMesh);
    
public slots:
    //! Create a physics world for a scene
    PhysicsWorld* CreatePhysicsWorldForScene(Scene::ScenePtr scene);
    
    //! Return the physics world for a scene if it exists
    PhysicsWorld* GetPhysicsWorldForScene(Scene::ScenePtr scene);
    
    //! Return the physics world for a scene if it exists
    PhysicsWorld* GetPhysicsWorldForScene(Scene::SceneManager* sceneraw);
    
    //! Enable/disable physics simulation
    void SetRunPhysics(bool enable);
    
    //! Enable/disable debug geometry
    void SetDrawDebugGeometry(bool enable);
    
    //! Get debug geometry enabled status
    bool GetDrawDebugGeometry() const { return drawDebugGeometry_; }
    
private slots:
    //! Scene has been removed, so delete also the physics world (if exists)
    void OnSceneRemoved(Scene::SceneManager* scene);
    
private:
    //! Update debug geometry manual object, if physics debug drawing is on
    void UpdateDebugGeometry();
    
    typedef std::map<Scene::SceneManager*, boost::shared_ptr<PhysicsWorld> > PhysicsWorldMap;
    //! Map of physics worlds assigned to scenes
    PhysicsWorldMap physicsWorlds_;
    
    typedef std::map<std::string, boost::shared_ptr<btTriangleMesh> > TriangleMeshMap;
    //! Bullet triangle meshes generated from Ogre meshes
    TriangleMeshMap triangleMeshes_;
    
    //! Debug geometry enabled flag
    bool drawDebugGeometry_;
    
    //! Manual object for the debug geometry
    Ogre::ManualObject* debugGeometryObject_;
    
    //! Whether should run physics. Default true
    bool runPhysics_;
    
    //! Bullet debug draw / debug behaviour flags
    int debugDrawMode_;
    
    //! Accept debug lines flag (ie. manual object is open for editing)
    /*! This is a safety for the case Bullet decides to call drawLine() at an unexpected time
     */ 
    bool acceptDebugLines_;
};

}

#endif
