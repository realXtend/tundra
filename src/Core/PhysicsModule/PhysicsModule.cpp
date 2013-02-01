// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "EC_RigidBody.h"
#include "EC_VolumeTrigger.h"
#include "EC_PhysicsMotor.h"
#include "OgreRenderingModule.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_Terrain.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Framework.h"
#include "Scene/Scene.h"
#include "Profiler.h"
#include "Renderer.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"
#include "QScriptEngineHelpers.h"
#include "LoggingFunctions.h"

// Disable unreferenced formal parameter coming from Bullet
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <QtScript>
#include <QTreeWidgetItem>

#include <Ogre.h>

#include "StaticPluginRegistry.h"

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(Physics::PhysicsModule*);
Q_DECLARE_METATYPE(Physics::PhysicsWorld*);
Q_DECLARE_METATYPE(PhysicsRaycastResult*);

namespace Physics
{

PhysicsModule::PhysicsModule()
:IModule("Physics"),
defaultPhysicsUpdatePeriod_(1.0f / 60.0f),
defaultMaxSubSteps_(6) // If fps is below 10, we start to slow down physics
{
}

PhysicsModule::~PhysicsModule()
{
}

void PhysicsModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_RigidBody>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_VolumeTrigger>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_PhysicsMotor>));
}

void PhysicsModule::Initialize()
{
    framework_->RegisterDynamicObject("physics", this);
    
    connect(framework_->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString&)));
    connect(framework_->Scene(), SIGNAL(SceneRemoved(const QString&)), this, SLOT(OnSceneRemoved(const QString&)));

    framework_->Console()->RegisterCommand("physicsdebug",
        "Toggles drawing of physics debug geometry.",
        this, SLOT(ToggleDebugGeometry()));
    framework_->Console()->RegisterCommand("stopphysics",
        "Stops physics simulation.",
        this, SLOT(StopPhysics()));
    framework_->Console()->RegisterCommand("startphysics",
        "(Re)starts physics simulation.",
        this, SLOT(StartPhysics()));
    framework_->Console()->RegisterCommand("autocollisionmesh",
        "Auto-assigns static rigid bodies with collision mesh to all visible meshes.",
        this, SLOT(AutoCollisionMesh()));
    
    // Check physics execution rate related command line parameters
    if (framework_->HasCommandLineParameter("--physicsrate"))
    {
        bool ok;
        int rate = framework_->CommandLineParameters("--physicsrate")[0].toInt(&ok);
        if (ok && rate > 0)
            SetDefaultPhysicsUpdatePeriod(1.0f / (float)rate);
    }
    if (framework_->HasCommandLineParameter("--physicsmaxsteps"))
    {
        bool ok;
        int steps = framework_->CommandLineParameters("--physicsmaxsteps")[0].toInt(&ok);
        if (ok && steps > 0)
            SetDefaultMaxSubSteps(steps);
    }
}

void PhysicsModule::Uninitialize()
{
}

void PhysicsModule::ToggleDebugGeometry()
{
    for (PhysicsWorldMap::iterator i = physicsWorlds_.begin(); i != physicsWorlds_.end(); ++i)
    {
        i->second->SetDebugGeometryEnabled(!i->second->IsDebugGeometryEnabled());
        i->second->drawDebugManuallySet_ = true; // Disable automatic debugdraw state change
    }
}

void PhysicsModule::SetDefaultPhysicsUpdatePeriod(float updatePeriod)
{
    // Allow max.1000 fps
    if (updatePeriod <= 0.001f)
        updatePeriod = 0.001f;
    defaultPhysicsUpdatePeriod_ = updatePeriod;
}

void PhysicsModule::SetDefaultMaxSubSteps(int steps)
{
    if (steps > 0)
        defaultMaxSubSteps_ = steps;
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
    Scene *scene = GetFramework()->Scene()->MainCameraScene();
    if (!scene)
    {
        LogError("No active scene!");
        return;
    }
    
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        // Only assign to entities that don't have a rigidbody yet, but have a mesh and a placeable
        if (!entity->GetComponent<EC_RigidBody>() && entity->GetComponent<EC_Placeable>() && entity->GetComponent<EC_Mesh>())
        {
            EC_RigidBody* body = checked_static_cast<EC_RigidBody*>(entity->GetOrCreateComponent(EC_RigidBody::TypeNameStatic(), "", AttributeChange::Default).get());
            body->SetShapeFromVisibleMesh();
        }
        // Terrain mode: assign if no rigid body, but there is a terrain component
        if (!entity->GetComponent<EC_RigidBody>() && entity->GetComponent<EC_Terrain>())
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
    
    shared_ptr<PhysicsWorld> newWorld = MAKE_SHARED(PhysicsWorld, scene, !scene->IsAuthority());
    newWorld->SetGravity(scene->UpVector() * -9.81f);
    newWorld->SetPhysicsUpdatePeriod(defaultPhysicsUpdatePeriod_);
    newWorld->SetMaxSubSteps(defaultMaxSubSteps_);
    physicsWorlds_[scene.get()] = newWorld;
    scene->setProperty(PhysicsWorld::PropertyName(), QVariant::fromValue<QObject*>(newWorld.get()));
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
        scene->setProperty(PhysicsWorld::PropertyName(), QVariant());
        physicsWorlds_.erase(scene.get());
    }
}

void PhysicsModule::SetRunPhysics(bool enable)
{
    for (PhysicsWorldMap::iterator i = physicsWorlds_.begin(); i != physicsWorlds_.end(); ++i)
        i->second->SetRunning(enable);
}

void PhysicsModule::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<Physics::PhysicsModule*>(engine);
    qScriptRegisterQObjectMetaType<Physics::PhysicsWorld*>(engine);
    qScriptRegisterQObjectMetaType<PhysicsRaycastResult*>(engine);
}

shared_ptr<btTriangleMesh> PhysicsModule::GetTriangleMeshFromOgreMesh(Ogre::Mesh* mesh)
{
    shared_ptr<btTriangleMesh> ptr;
    if (!mesh)
        return ptr;
    
    // Check if has already been converted
    TriangleMeshMap::const_iterator iter = triangleMeshes_.find(mesh->getName());
    if (iter != triangleMeshes_.end())
        return iter->second;
    
    // Create new, then interrogate the Ogre mesh
#include "DisableMemoryLeakCheck.h"
    ptr = MAKE_SHARED(btTriangleMesh);
#include "EnableMemoryLeakCheck.h"
    GenerateTriangleMesh(mesh, ptr.get());
    
    triangleMeshes_[mesh->getName()] = ptr;
    
    return ptr;
}

shared_ptr<ConvexHullSet> PhysicsModule::GetConvexHullSetFromOgreMesh(Ogre::Mesh* mesh)
{
    shared_ptr<ConvexHullSet> ptr;
    if (!mesh)
        return ptr;
    
    // Check if has already been converted
    ConvexHullSetMap::const_iterator iter = convexHullSets_.find(mesh->getName());
    if (iter != convexHullSets_.end())
        return iter->second;
    
    // Create new, then interrogate the Ogre mesh
    ptr = MAKE_SHARED(ConvexHullSet);
    GenerateConvexHullSet(mesh, ptr.get());

    convexHullSets_[mesh->getName()] = ptr;
    
    return ptr;
}

#ifdef PROFILING
static QTreeWidgetItem *FindItemByName(QTreeWidgetItem *parent, const char *name)
{
    for(int i = 0; i < parent->childCount(); ++i)
    {
        if (parent->child(i)->text(0) == name)
            return parent->child(i);
        QTreeWidgetItem *item = FindItemByName(parent->child(i), name);
        if (item)
            return item;
    }

    return 0;
}

void UpdateBulletProfilingData(CProfileIterator* profileIterator, QTreeWidgetItem *parentItem, int numFrames)
{
	profileIterator->First();
	if (profileIterator->Is_Done())
		return;

//	float accumulated_time=0;
//    float parent_time = profileIterator->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : profileIterator->Get_Current_Parent_Total_Time();
	int i;
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();

//    const char *bulletParentNodeName = profileIterator->Get_Current_Parent_Name();
//	printf("Profiling: %s (total running time: %.3f ms) ---\n",	profileIterator->Get_Current_Parent_Name(), parent_time );

    std::vector<QTreeWidgetItem*> itemsThisLevel; // Cache items at this level to a vector, since bullet profiling iterator requires us to do two passes.
	int numChildren = 0;
    for(int i = 0; !profileIterator->Is_Done(); i++,profileIterator->Next())
	{
        ++numChildren;
		float current_total_time = profileIterator->Get_Current_Total_Time();
//		accumulated_time += current_total_time;
//		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;
		//printf("%d -- %s (%.2f %%) :: %.3f ms / frame (%d calls)\n",i, profileIterator->Get_Current_Name(), fraction,(current_total_time / (double)frames_since_reset),profileIterator->Get_Current_Total_Calls());
//		totalTime += current_total_time;
		//recurse into children

        QTreeWidgetItem *item = FindItemByName(parentItem, profileIterator->Get_Current_Name());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(profileIterator->Get_Current_Name()));
            parentItem->addChild(item);
        }
        itemsThisLevel.push_back(item);
        char str[256];
        item->setText(1, QString("%1").arg(frames_since_reset));
        sprintf(str, "%.2f", (float)frames_since_reset / numFrames);
        item->setText(2, str);
        item->setText(3, "?");
        sprintf(str, "%.2fms", (float)current_total_time / profileIterator->Get_Current_Total_Calls());
        item->setText(4, str);
        sprintf(str, "%.2fms", (float)current_total_time / frames_since_reset);
        item->setText(5, str);
        item->setText(6, "?");
    }

	for (i=0;i<numChildren;i++)
    {
		profileIterator->Enter_Child(i);
		UpdateBulletProfilingData(profileIterator, itemsThisLevel[i], numFrames);
		profileIterator->Enter_Parent();
	}
}

void UpdateBulletProfilingData(QTreeWidgetItem *treeRoot, int numFrames)
{
    QTreeWidgetItem *bulletRootNode = FindItemByName(treeRoot, "Bullet_stepSimulation");
    if (!bulletRootNode)
        return; // We've lost the physics world update node, or no physics occurring, skip bullet profiling altogether.

	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

    UpdateBulletProfilingData(profileIterator, bulletRootNode, numFrames);

	CProfileManager::Release_Iterator(profileIterator);
}
#endif

}

using namespace Physics;

extern "C"
{
#ifndef ANDROID
DLLEXPORT void TundraPluginMain(Framework *fw)
#else
DEFINE_STATIC_PLUGIN_MAIN(PhysicsModule)
#endif
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new Physics::PhysicsModule();
    fw->RegisterModule(module);
}
}
