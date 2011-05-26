// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "EC_RigidBody.h"
#include "EC_VolumeTrigger.h"
#include "OgreRenderingModule.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_Terrain.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Framework.h"
#include "Scene.h"
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
:IModule("Physics")
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
    
    connect(framework_->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString&)));
    connect(framework_->Scene(), SIGNAL(SceneRemoved(const QString&)), this, SLOT(OnSceneRemoved(const QString&)));
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
}

void PhysicsModule::ToggleDebugGeometry()
{
    for (PhysicsWorldMap::iterator i = physicsWorlds_.begin(); i != physicsWorlds_.end(); ++i)
    {
        i->second->SetDrawDebugGeometry(!i->second->GetDrawDebugGeometry());
        i->second->drawDebugManuallySet_ = true; // Disable automatic debugdraw state change
    }
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
    
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
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
    PROFILE(PhysicsModule_Update);
    // Loop all the physics worlds and update them.
    PhysicsWorldMap::iterator i = physicsWorlds_.begin();
    while(i != physicsWorlds_.end())
    {
        i->second->Simulate(frametime);
        ++i;
    }
}

void PhysicsModule::OnSceneAdded(const QString& name)
{
    ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    if (!scene)
    {
        LogError("Could not find created scene");
        return;
    }
    
    boost::shared_ptr<PhysicsWorld> newWorld(new PhysicsWorld(scene, !scene->IsAuthority()));
    newWorld->SetGravity(scene->GetUpVector() * -9.81f);
    physicsWorlds_[scene.get()] = newWorld;
    scene->setProperty(PhysicsWorld::PropertyNameStatic(), QVariant::fromValue<QObject*>(newWorld.get()));
}

void PhysicsModule::OnSceneRemoved(const QString& name)
{
    // Remove the PhysicsWorld from the scene
    ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    if (!scene)
    {
        LogError("Could not find scene about to be removed");
        return;
    }
    
    PhysicsWorld* worldPtr = scene->GetWorld<PhysicsWorld>().get();
    if (worldPtr)
    {
        scene->setProperty(PhysicsWorld::PropertyNameStatic(), QVariant());
        physicsWorlds_.erase(scene.get());
    }
}

void PhysicsModule::SetRunPhysics(bool enable)
{
    for (PhysicsWorldMap::iterator i = physicsWorlds_.begin(); i != physicsWorlds_.end(); ++i)
        i->second->SetRunPhysics(enable);
}

void PhysicsModule::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<Physics::PhysicsModule*>(engine);
    qScriptRegisterQObjectMetaType<Physics::PhysicsWorld*>(engine);
    qScriptRegisterQObjectMetaType<PhysicsRaycastResult*>(engine);
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
