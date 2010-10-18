// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_RigidBody.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
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
    
    for(uint i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
        const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
        unsigned char* vertices = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        uint vertexSize = vbuf->getVertexSize();
        float* pReal = 0;
        
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        
        if (use32bitindexes)
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pLong[k];
                uint i2 = pLong[k+1];
                uint i3 = pLong[k+2];
                
                //! Haxor the collision mesh for the Ogre->Opensim coordinate space adjust
                /*! \todo Hopefully the need for this is eliminated soon
                 */
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                btVector3 v1(-pReal[0], pReal[2], pReal[1]);
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                btVector3 v2(-pReal[0], pReal[2], pReal[1]);
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                btVector3 v3(-pReal[0], pReal[2], pReal[1]);
                
                ptr->addTriangle(v1, v2, v3);
            }
        }
        else
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pShort[k];
                uint i2 = pShort[k+1];
                uint i3 = pShort[k+2];
                
                //! Haxor the collision mesh for the Ogre->Opensim coordinate space adjust
                /*! \todo Hopefully the need for this is eliminated soon
                 */
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                btVector3 v1(-pReal[0], pReal[2], pReal[1]);
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                btVector3 v2(-pReal[0], pReal[2], pReal[1]);
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                btVector3 v3(-pReal[0], pReal[2], pReal[1]);
                
                ptr->addTriangle(v1, v2, v3);
            }
        }
        
        vbuf->unlock();
        ibuf->unlock();
    }
    
    triangleMeshes_[mesh->getName()] = ptr;
    
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
