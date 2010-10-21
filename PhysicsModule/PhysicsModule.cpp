// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_RigidBody.h"
#include "EC_Terrain.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "MemoryLeakCheck.h"
#include "Entity.h"
#include "Framework.h"
#include "SceneManager.h"
#include "ServiceManager.h"
#include "Profiler.h"
#include "Renderer.h"
#include "ConsoleCommandServiceInterface.h"
#include "btBulletDynamicsCommon.h"

#include <Ogre.h>

namespace Physics
{

const std::string PhysicsModule::moduleName = std::string("Physics");

PhysicsModule::PhysicsModule() :
    IModule(NameStatic()),
    drawDebugGeometry_(false),
    acceptDebugLines_(false),
    runPhysics_(true),
    debugGeometryObject_(0),
    debugDrawMode_(0)
{
}

PhysicsModule::~PhysicsModule()
{
    // Delete the physics manual object if it exists
    SetDrawDebugGeometry(false);
}

void PhysicsModule::Load()
{
    DECLARE_MODULE_EC(EC_RigidBody);
}

void PhysicsModule::PostInitialize()
{
    RegisterConsoleCommand(Console::CreateCommand("physicsdebug",
        "Toggles drawing of physics debug geometry.",
        Console::Bind(this, &PhysicsModule::ConsoleToggleDebugGeometry)));
    RegisterConsoleCommand(Console::CreateCommand("stopphysics",
        "Stops physics simulation.",
        Console::Bind(this, &PhysicsModule::ConsoleStopPhysics)));
    RegisterConsoleCommand(Console::CreateCommand("startphysics",
        "(Re)starts physics simulation.",
        Console::Bind(this, &PhysicsModule::ConsoleStartPhysics)));
    RegisterConsoleCommand(Console::CreateCommand("autocollisionmesh",
        "Auto-assigns static rigid bodies with collision mesh to all visible meshes.",
        Console::Bind(this, &PhysicsModule::ConsoleAutoCollisionMesh)));
}

Console::CommandResult PhysicsModule::ConsoleToggleDebugGeometry(const StringVector& params)
{
    SetDrawDebugGeometry(!drawDebugGeometry_);
    
    return Console::ResultSuccess();
}

Console::CommandResult PhysicsModule::ConsoleStopPhysics(const StringVector& params)
{
    SetRunPhysics(false);
    
    return Console::ResultSuccess();
}

Console::CommandResult PhysicsModule::ConsoleStartPhysics(const StringVector& params)
{
    SetRunPhysics(true);
    
    return Console::ResultSuccess();
}

Console::CommandResult PhysicsModule::ConsoleAutoCollisionMesh(const StringVector& params)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene");
    
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::EntityPtr entity = iter->second;
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
    
    return Console::ResultSuccess();
}

void PhysicsModule::Update(f64 frametime)
{
    if (runPhysics_)
    {
        PROFILE(PhysicsModule_Update);
        // Loop all the physics worlds and update them.
        PhysicsWorldMap::iterator i = physicsWorlds_.begin();
        while (i != physicsWorlds_.end())
        {
            i->second->Simulate(frametime);
            ++i;
        }
        
        if (drawDebugGeometry_)
            UpdateDebugGeometry();
    }
    
    RESETPROFILER;
}

PhysicsWorld* PhysicsModule::CreatePhysicsWorldForScene(Scene::ScenePtr scene)
{
    if (!scene)
        return 0;
    
    PhysicsWorld* old = GetPhysicsWorldForScene(scene);
    if (old)
    {
        LogWarning("Physics world already exists for scene");
        return old;
    }
    
    Scene::SceneManager* ptr = scene.get();
    boost::shared_ptr<PhysicsWorld> new_world(new PhysicsWorld(this));
    physicsWorlds_[ptr] = new_world;
    QObject::connect(ptr, SIGNAL(Removed(Scene::SceneManager*)), this, SLOT(OnSceneRemoved(Scene::SceneManager*)));
    
    LogInfo("Created new physics world");
    
    return new_world.get();
}

PhysicsWorld* PhysicsModule::GetPhysicsWorldForScene(Scene::SceneManager* sceneraw)
{
    if (!sceneraw)
        return 0;
    
    PhysicsWorldMap::iterator i = physicsWorlds_.find(sceneraw);
    if (i == physicsWorlds_.end())
        return 0;
    return i->second.get();
}

PhysicsWorld* PhysicsModule::GetPhysicsWorldForScene(Scene::ScenePtr scene)
{
    return GetPhysicsWorldForScene(scene.get());
}

void PhysicsModule::OnSceneRemoved(Scene::SceneManager* scene)
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
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;
    Ogre::SceneManager* scenemgr = renderer->GetSceneManager();
    
    drawDebugGeometry_ = enable;
    if (!enable)
    {
        if (debugGeometryObject_)
        {
            scenemgr->getRootSceneNode()->detachObject(debugGeometryObject_);
            scenemgr->destroyManualObject(debugGeometryObject_);
            debugGeometryObject_ = 0;
        }
    }
    else
    {
        if (!debugGeometryObject_)
        {
            debugGeometryObject_ = scenemgr->createManualObject("physics_debug");
            debugGeometryObject_->setDynamic(true);
            scenemgr->getRootSceneNode()->attachObject(debugGeometryObject_);
        }
    }
}

void PhysicsModule::UpdateDebugGeometry()
{
    if (!drawDebugGeometry_)
        return;

    PROFILE(PhysicsModule_UpdateDebugGeometry);

    setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    // Draw debug only for the active scene
    PhysicsWorld* world = GetPhysicsWorldForScene(framework_->GetDefaultWorldScene());
    if (!world)
        return;
    
    debugGeometryObject_->clear();
    debugGeometryObject_->begin("PhysicsDebug", Ogre::RenderOperation::OT_LINE_LIST);
    acceptDebugLines_ = true;
    
    world->GetWorld()->debugDrawWorld();

    acceptDebugLines_ = false;
    debugGeometryObject_->end();
}

void PhysicsModule::reportErrorWarning(const char* warningString)
{
    LogWarning("Physics: " + std::string(warningString));
}

void PhysicsModule::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if (!acceptDebugLines_)
        return;
    
    if ((drawDebugGeometry_) && (debugGeometryObject_))
    {
        debugGeometryObject_->position(from.x(), from.y(), from.z());
        debugGeometryObject_->colour(color.x(), color.y(), color.z(), 1.0f);
        debugGeometryObject_->position(to.x(), to.y(), to.z());
        debugGeometryObject_->colour(color.x(), color.y(), color.z(), 1.0f);
    }
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
    ptr = boost::shared_ptr<btTriangleMesh>(new btTriangleMesh());
    GenerateTriangleMesh(mesh, ptr.get(), true);

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
    GenerateConvexHullSet(mesh, ptr.get(), true);

    convexHullSets_[mesh->getName()] = ptr;
    
    return ptr;
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Physics;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(PhysicsModule)
POCO_END_MANIFEST 
