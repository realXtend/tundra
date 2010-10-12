// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "MemoryLeakCheck.h"
#include "Entity.h"
#include "Framework.h"
#include "SceneManager.h"

#include "btBulletDynamicsCommon.h"

namespace Physics
{

const std::string PhysicsModule::moduleName = std::string("Physics");

PhysicsModule::PhysicsModule() : 
    IModule(NameStatic()),
    physicsUpdatePeriod_(1.0f / 60.0f)
{
}

PhysicsModule::~PhysicsModule()
{
}

void PhysicsModule::PostInitialize()
{
}

void PhysicsModule::Update(f64 frametime)
{
    // Loop all the physics worlds and update them.
    PhysicsWorldMap::iterator i = physicsWorlds_.begin();
    while (i != physicsWorlds_.end())
    {
        // Assume we generate at least 10 frames per second. If less, the physics will start to slow down.
        float minFps = 10;
        
        int maxSubSteps = (int)(physicsUpdatePeriod_ / minFps);
        i->second->Simulate((float)frametime, maxSubSteps, physicsUpdatePeriod_);
        
        ++i;
    }
}

void PhysicsModule::SetPhysicsUpdatePeriod(float updatePeriod)
{
    physicsUpdatePeriod_ = updatePeriod;
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
    boost::shared_ptr<PhysicsWorld> new_world(new PhysicsWorld());
    physicsWorlds_[ptr] = new_world;
    QObject::connect(ptr, SIGNAL(destroyed(QObject*)), this, SLOT(OnSceneRemoved(QObject*)));
    
    return new_world.get();
}

PhysicsWorld* PhysicsModule::GetPhysicsWorldForScene(Scene::ScenePtr scene)
{
    if (!scene)
        return 0;
    
    Scene::SceneManager* ptr = scene.get();
    PhysicsWorldMap::iterator i = physicsWorlds_.find(ptr);
    if (i == physicsWorlds_.end())
        return 0;
    return i->second.get();
}

void PhysicsModule::OnSceneRemoved(QObject* scene)
{
    Scene::SceneManager* ptr = dynamic_cast<Scene::SceneManager*>(scene);
    PhysicsWorldMap::iterator i = physicsWorlds_.find(ptr);
    if (i != physicsWorlds_.end())
        physicsWorlds_.erase(i);
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
