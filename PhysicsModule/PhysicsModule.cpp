// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "EC_RigidBody.h"
#include "EC_VolumeTrigger.h"
#include "OgreBulletCollisionsDebugLines.h"
#include "OgreRenderingModule.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_Terrain.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Framework.h"
#include "SceneManager.h"
#include "Profiler.h"
#include "Renderer.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"

#include <btBulletDynamicsCommon.h>

#include <QtScript>

#include <Ogre.h>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(Physics::PhysicsModule*);
Q_DECLARE_METATYPE(Physics::PhysicsWorld*);
Q_DECLARE_METATYPE(PhysicsRaycastResult*);

// The following functions help register a custom QObject-derived class to a QScriptEngine.
// See http://lists.trolltech.com/qt-interest/2007-12/thread00158-0.html .
template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{   
    qobject = qobject_cast<Tp>(value.toQObject());
}

template <typename Tp>
int qScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , Tp * = 0
#endif
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject,
                                       qScriptValueToQObject, prototype);
}


namespace Physics
{

PhysicsModule::PhysicsModule()
:IModule("Physics"),
drawDebugGeometry_(false),
drawDebugManuallySet_(false),
runPhysics_(true),
debugGeometryObject_(0),
debugDrawMode_(0)
{
}

PhysicsModule::~PhysicsModule()
{
}

void PhysicsModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_RigidBody>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_VolumeTrigger>));
}

void PhysicsModule::Initialize()
{
    framework_->RegisterDynamicObject("physics", this);
}

void PhysicsModule::PostInitialize()
{
    framework_->Console()->RegisterCommand("physicsdebug",
        "Toggles drawing of physics debug geometry.",
        this, SLOT(ConsoleToggleDebugGeometry()));
    framework_->Console()->RegisterCommand("stopphysics",
        "Stops physics simulation.",
        this, SLOT(ConsoleStopPhysics()));
    framework_->Console()->RegisterCommand("startphysics",
        "(Re)starts physics simulation.",
        this, SLOT(ConsoleStartPhysics()));
    framework_->Console()->RegisterCommand("autocollisionmesh",
        "Auto-assigns static rigid bodies with collision mesh to all visible meshes.",
        this, SLOT(ConsoleAutoCollisionMesh()));
}

void PhysicsModule::Uninitialize()
{
    // Delete the physics debug object if it exists
    SetDrawDebugGeometry(false);
}

void PhysicsModule::ToggleDebugGeometry()
{
    SetDrawDebugGeometry(!drawDebugGeometry_);
    drawDebugManuallySet_ = true; // Disable automatic debugdraw state change
}

void PhysicsModule::StopPhysics()
{
    SetRunPhysics(false);
}

void PhysicsModule::StartPhysics()
{
    SetRunPhysics(true);
}

void PhysicsModule::AutoCollisionMesh()
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogError("No active scene!");
        return;
    }
    
    for(SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        // Only assign to entities that don't have a rigidbody yet, but have a mesh and a placeable
        if ((!entity->GetComponent<EC_RigidBody>()) && (entity->GetComponent<EC_Placeable>()) && (entity->GetComponent<EC_Mesh>()))
        {
            EC_RigidBody* body = checked_static_cast<EC_RigidBody*>(entity->GetOrCreateComponent(EC_RigidBody::TypeNameStatic(), "", AttributeChange::Default).get());
            body->SetShapeFromVisibleMesh();
        }
        // Terrain mode: assign if no rigid body, but there is a terrain component
        if ((!entity->GetComponent<EC_RigidBody>()) && (entity->GetComponent<Environment::EC_Terrain>()))
        {
            EC_RigidBody* body = checked_static_cast<EC_RigidBody*>(entity->GetOrCreateComponent(EC_RigidBody::TypeNameStatic(), "", AttributeChange::Default).get());
            body->shapeType.Set(EC_RigidBody::Shape_HeightField, AttributeChange::Default);
        }
    }
}

void PhysicsModule::Update(f64 frametime)
{
    if (runPhysics_)
    {
        PROFILE(PhysicsModule_Update);
        // Loop all the physics worlds and update them.
        PhysicsWorldMap::iterator i = physicsWorlds_.begin();
        while(i != physicsWorlds_.end())
        {
            i->second->Simulate(frametime);
            ++i;
        }
        
        // Automatically enable debug geometry if at least one debug-enabled rigidbody. Automatically disable if no debug-enabled rigidbodies
        // However, do not do this if user has used the physicsdebug console command
        if (!drawDebugManuallySet_)
        {
            if ((!drawDebugGeometry_) && (!debugRigidBodies_.empty()))
                SetDrawDebugGeometry(true);
            if ((drawDebugGeometry_) && (debugRigidBodies_.empty()))
                SetDrawDebugGeometry(false);
        }
        
        if (drawDebugGeometry_)
            UpdateDebugGeometry();
    }
    
}

Physics::PhysicsWorld* PhysicsModule::CreatePhysicsWorldForScene(ScenePtr scene, bool isClient)
{
    if (!scene)
        return 0;
    
    PhysicsWorld* old = GetPhysicsWorldForScene(scene);
    if (old)
    {
        LogWarning("Physics world already exists for scene");
        return old;
    }
    
    SceneManager* ptr = scene.get();
    boost::shared_ptr<PhysicsWorld> new_world(new PhysicsWorld(this, isClient));
    new_world->SetGravity(scene->GetUpVector() * -9.81f);
    
    physicsWorlds_[ptr] = new_world;
    QObject::connect(ptr, SIGNAL(Removed(SceneManager*)), this, SLOT(OnSceneRemoved(SceneManager*)));
    
    LogDebug("Created new physics world");
    
    return new_world.get();
}

Physics::PhysicsWorld* PhysicsModule::GetPhysicsWorldForScene(SceneManager* sceneraw)
{
    if (!sceneraw)
        return 0;
    
    PhysicsWorldMap::iterator i = physicsWorlds_.find(sceneraw);
    if (i == physicsWorlds_.end())
        return 0;
    return i->second.get();
}

Physics::PhysicsWorld* PhysicsModule::GetPhysicsWorldForScene(ScenePtr scene)
{
    return GetPhysicsWorldForScene(scene.get());
}

Physics::PhysicsWorld* PhysicsModule::GetPhysicsWorld(QObject* scene)
{
    return GetPhysicsWorldForScene(dynamic_cast<SceneManager*>(scene));
}

void PhysicsModule::OnSceneRemoved(SceneManager* scene)
{
    PhysicsWorldMap::iterator i = physicsWorlds_.find(scene);
    if (i != physicsWorlds_.end())
    {
        LogInfo("Scene removed, removing physics world");
        physicsWorlds_.erase(i);
    }
}

void PhysicsModule::SetRunPhysics(bool enable)
{
    runPhysics_ = enable;
}

void PhysicsModule::SetDrawDebugGeometry(bool enable)
{
    if (!framework_ || framework_->IsHeadless() || drawDebugGeometry_ == enable)
        return;

    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!renderer)
        return;
    Ogre::SceneManager* scenemgr = renderer->GetSceneManager();
    
    drawDebugGeometry_ = enable;
    if (!enable)
    {
        setDebugMode(0);
        
        if (debugGeometryObject_)
        {
            scenemgr->getRootSceneNode()->detachObject(debugGeometryObject_);
            delete debugGeometryObject_;
            debugGeometryObject_ = 0;
        }
    }
    else
    {
        setDebugMode(btIDebugDraw::DBG_DrawWireframe);
        
        if (!debugGeometryObject_)
        {
#include "DisableMemoryLeakCheck.h"
            debugGeometryObject_ = new DebugLines();
#include "EnableMemoryLeakCheck.h"
            scenemgr->getRootSceneNode()->attachObject(debugGeometryObject_);
        }
    }
}

void PhysicsModule::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<Physics::PhysicsModule*>(engine);
    qScriptRegisterQObjectMetaType<Physics::PhysicsWorld*>(engine);
    qScriptRegisterQObjectMetaType<PhysicsRaycastResult*>(engine);
}

void PhysicsModule::UpdateDebugGeometry()
{
    if ((!drawDebugGeometry_) || (!debugGeometryObject_))
        return;

    PROFILE(PhysicsModule_UpdateDebugGeometry);

    // Draw debug only for the active scene
    PhysicsWorld* world = GetPhysicsWorldForScene(GetFramework()->Scene()->GetDefaultScene());
    if (!world)
        return;
    
    // Get all lines of the physics world
    world->GetWorld()->debugDrawWorld();
    
    // Build the debug vertex buffer. Note: this is a no-op if there is no debug objects to draw
    debugGeometryObject_->draw();
}

void PhysicsModule::reportErrorWarning(const char* warningString)
{
    LogWarning("Physics: " + std::string(warningString));
}

void PhysicsModule::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if ((drawDebugGeometry_) && (debugGeometryObject_))
        debugGeometryObject_->addLine(from, to, color);
}

boost::shared_ptr<btTriangleMesh> PhysicsModule::GetTriangleMeshFromOgreMesh(Ogre::Mesh* mesh)
{
    boost::shared_ptr<btTriangleMesh> ptr;
    if (!mesh)
        return ptr;
    
    // Check if has already been converted
    TriangleMeshMap::const_iterator iter = triangleMeshes_.find(mesh->getName());
    if (iter != triangleMeshes_.end())
        return iter->second;
    
    // Create new, then interrogate the Ogre mesh
#include "DisableMemoryLeakCheck.h"
    ptr = boost::shared_ptr<btTriangleMesh>(new btTriangleMesh());
#include "EnableMemoryLeakCheck.h"
    GenerateTriangleMesh(mesh, ptr.get());
    
    triangleMeshes_[mesh->getName()] = ptr;
    
    return ptr;
}

boost::shared_ptr<ConvexHullSet> PhysicsModule::GetConvexHullSetFromOgreMesh(Ogre::Mesh* mesh)
{
    boost::shared_ptr<ConvexHullSet> ptr;
    if (!mesh)
        return ptr;
    
    // Check if has already been converted
    ConvexHullSetMap::const_iterator iter = convexHullSets_.find(mesh->getName());
    if (iter != convexHullSets_.end())
        return iter->second;
    
    // Create new, then interrogate the Ogre mesh
    ptr = boost::shared_ptr<ConvexHullSet>(new ConvexHullSet());
    GenerateConvexHullSet(mesh, ptr.get());

    convexHullSets_[mesh->getName()] = ptr;
    
    return ptr;
}

}

using namespace Physics;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new Physics::PhysicsModule();
    fw->RegisterModule(module);
}
}
