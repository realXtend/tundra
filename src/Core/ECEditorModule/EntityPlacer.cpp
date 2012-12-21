// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EntityPlacer.h"
#include "Scene/Scene.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "InputAPI.h"
#include "EC_Mesh.h"
#include "OgreRenderingModule.h"

#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>

#include "MemoryLeakCheck.h"

EntityPlacer::EntityPlacer(Framework *framework, entity_id_t entityId, QObject *parent):
    framework_(framework),
    QObject(parent),
    finished_(false),
    useCustomMesh_(false),
    meshEntity_(0),
    previousScrollValue_(0),
    location_(float3::zero),
    orientation_(Quat::identity)
{
    static const std::string customMeshName("Selection.mesh");
    input_ = framework_->Input()->RegisterInputContext("EntityPlacement", 110);
    
    Scene* scene = framework_->Scene()->MainCameraScene();
    entity_ = scene->GetEntity(entityId);
    if(!entity_.expired())
    {
        Entity *entity = entity_.lock().get();
        placeable_ = entity->GetComponent<EC_Placeable>().get();
        if(!placeable_)
            return;

        // If entity dont hold any visual element create a custom mesh.
        if (!entity->GetComponent("EC_Mesh"))
        {
            useCustomMesh_ = true;

            world_ = scene->GetWorld<OgreWorld>();
            if (world_.expired())
                return;
            OgreWorldPtr world = world_.lock();
            
            Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
            meshEntity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EntityPlacer"), customMeshName);
            meshEntity_->getSubEntity(0)->setMaterialName("Clone");
            meshEntity_->setCastShadows(false);
            placeable_->GetSceneNode()->attachObject(meshEntity_);
        }
    }

    input_->SetTakeMouseEventsOverQt(true);

    connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), this, SLOT(OnMouseEvent(MouseEvent *)));
    connect(input_.get(), SIGNAL(MouseMove(MouseEvent *)), this, SLOT(MouseMove(MouseEvent *)));
}

EntityPlacer::~EntityPlacer()
{
    if(useCustomMesh_)
    {
        if (world_.expired())
            return;
        if(meshEntity_)
        {
            world_.lock()->OgreSceneManager()->destroyEntity(meshEntity_);
            meshEntity_ = 0;
        }
    }
}

void EntityPlacer::OnMouseEvent(MouseEvent *mouse)
{
    if(mouse->eventType == MouseEvent::MousePressed && mouse->button == MouseEvent::LeftButton)
    {
        deleteLater();
        emit Finished(location_, orientation_);
        finished_ =  true;
    }
    else if(mouse->eventType == MouseEvent::MouseScroll)
    {
        /// @Todo add scele code inside.
    }
}

void EntityPlacer::MouseMove(MouseEvent *mouse)
{
    float3 result;
    if(DoRayCast(mouse->x, mouse->y, result) && !finished_)
    {
        location_ = result;
        if(placeable_)
            placeable_->SetPosition(location_);
    }
}

bool EntityPlacer::DoRayCast(int x, int y, float3 &result)
{
    // do raycast into the world when user is dragging the mouse while hes holding left button down.
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!renderer)
        return false;

    RaycastResult* cast_result = renderer->Raycast(x, y);
    Entity *entity = cast_result->entity;
    if (!entity) // User didn't click on terrain or other entities.
        return false;

    result = cast_result->pos;
    return true;
}
