#include "StableHeaders.h"
#include "EntityPlacer.h"
#include "SceneManager.h"

#include "EC_Placeable.h"
#include "Renderer.h"
#include "InputServiceInterface.h"
#include "EC_Mesh.h"
#include "OgreRenderingModule.h"
#include <Ogre.h>

namespace ECEditor
{
EntityPlacer::EntityPlacer(Foundation::Framework *framework, entity_id_t entityId, QObject *parent):
    framework_(framework),
    QObject(parent),
    finnished_(false),
    useCustomMesh_(false),
    meshEntity_(0),
    previousScrollValue_(0)
{
    static const std::string customMeshName("Selection.mesh");
    input_ = framework_->Input()->RegisterInputContext("EntityPlacement", 110);
    
    entity_ = framework_->GetDefaultWorldScene()->GetEntity(entityId);
    if(!entity_.expired())
    {
        Scene::Entity *entity = entity_.lock().get();
        placeable_ = entity->GetComponent<EC_Placeable>().get();
        if(!placeable_)
            return;

        // If entity dont hold any visual element create a custom mesh.
        if(entity->HasComponent("EC_Mesh") == false)
        {
            useCustomMesh_ = true;
            OgreRenderer::OgreRenderingModule *rendererModule = framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock().get();
            if(!rendererModule)
                return;

            renderer_ = rendererModule->GetRenderer();
            if(renderer_.expired())
                return;

            Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
            meshEntity_ = scene_mgr->createEntity(renderer_.lock()->GetUniqueObjectName(), customMeshName);
            meshEntity_->getSubEntity(0)->setMaterialName("Clone");
            meshEntity_->setCastShadows(false);
            placeable_->GetSceneNode()->attachObject(meshEntity_);
        }
    }

    input_->SetTakeMouseEventsOverQt(true);

    connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), this, SLOT(OnMouseEvent(MouseEvent *)));
    connect(input_.get(), SIGNAL(MouseMove(MouseEvent *)), this, SLOT(MouseMove(MouseEvent *)));
}

EntityPlacer::~EntityPlacer()
{
    if(useCustomMesh_)
    {
        if (renderer_.expired())
            return;
        OgreRenderer::RendererPtr renderer = renderer_.lock();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();

        if(meshEntity_)
        {
            scene_mgr->destroyEntity(meshEntity_);
            meshEntity_ = 0;
        }
    }
}

void EntityPlacer::OnMouseEvent(MouseEvent *mouse)
{
    if(mouse->eventType == MouseEvent::MousePressed && mouse->button == MouseEvent::LeftButton)
    {
        deleteLater();
        emit Finnished(location_, orientation_);
        finnished_ =  true;
    }
    else if(mouse->eventType == MouseEvent::MouseScroll)
    {
        //! @Todo add scele code inside.
    }
}

void EntityPlacer::MouseMove(MouseEvent *mouse)
{
    Vector3df result;
    if(DoRayCast(mouse->x, mouse->y, result) && !finnished_)
    {
        location_ = result;
        if(placeable_)
            placeable_->SetPosition(location_);
    }
}

bool EntityPlacer::DoRayCast(int x, int y, Vector3df &result)
{
    // do raycast into the world when user is dragging the mouse while hes holding left button down.
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return false;

    Foundation::RaycastResult cast_result = renderer->Raycast(x, y);
    Scene::Entity *entity = cast_result.entity_;
    if (!entity) // User didn't click on terrain or other entities.
        return false;

    result = cast_result.pos_;
    return true;
}
}