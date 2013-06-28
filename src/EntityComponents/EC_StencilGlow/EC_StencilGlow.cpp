/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_StencilGlow.h
 *  @brief  Adds an outline to a mesh.
 */

#include "EC_StencilGlow.h"
#include "Scene.h"
#include "Entity.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#define COLOR_CUSTOM_PARAM 1

EC_StencilGlow::EC_StencilGlow(Scene *scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(enabled, "Enabled", false),
    INIT_ATTRIBUTE_VALUE(color, "Color", Color(1.f, 1.f, 1.f, 0.4f)),
    isEnabled(false),
    outlineEntity_(0),
    outlineSceneNode_(0)
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Initialize()));
}

EC_StencilGlow::~EC_StencilGlow()
{
    DestroyStencilGlow();
}

void EC_StencilGlow::Initialize()
{
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    world_ = ParentScene()->Subsystem<OgreWorld>();

    EC_Mesh* mesh = GetMesh();
    if (!mesh)
    {
        LogError("EC_StencilGlow needs EC_Mesh in advance in order to set it!");
        return;
    }

    connect(mesh, SIGNAL(MeshChanged()), this, SLOT(OnMeshChanged()));
    connect(mesh, SIGNAL(MeshAboutToBeDestroyed()), this, SLOT(OnMeshAboutToBeDestroyed()));
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(OnAttributeUpdated(IAttribute*)));

    if (enabled.Get())
    {
        CreateStencilGlow();
        SetStencilGlowEnabled(true);
    }
}

void EC_StencilGlow::CreateStencilGlow()
{
    if (!ViewEnabled())
        return;

    if (world_.expired())
        return;

    EC_Mesh* mesh = GetMesh();
    if (!mesh)
    {
        LogError("EC_StencilGlow needs EC_Mesh in advance in order to set it!");
        return;
    }

    Ogre::Entity* entity = mesh->GetEntity();
    if (!entity)
        return;

    if (!outlineEntity_ && !outlineSceneNode_)
    {
        outlineEntity_ = entity->clone(entity->getName() + "_glow");
        outlineEntity_->setRenderQueueGroup(STENCIL_GLOW_OUTLINE);
        outlineEntity_->setMaterialName("cg/stencil_glow");

        if (entity->hasSkeleton())
            outlineEntity_->shareSkeletonInstanceWith(entity);
        
        Ogre::SceneManager* mgr = world_.lock()->OgreSceneManager();
        if (mgr)
        {
            outlineSceneNode_ = entity->getParentSceneNode()->createChildSceneNode(entity->getName() + "_outlineGlowNode");
            outlineSceneNode_->setScale(Ogre::Vector3(1.08f, 1.08f, 1.08f));
        }
    }
}

void EC_StencilGlow::DestroyStencilGlow()
{
    if (world_.expired())
        return;

    if (outlineEntity_)
    {
        world_.lock()->OgreSceneManager()->destroyEntity(outlineEntity_);
        outlineEntity_ = 0;
    }

    if (outlineSceneNode_)
    {
        world_.lock()->OgreSceneManager()->destroySceneNode(outlineSceneNode_);
        outlineSceneNode_ = 0;
    }
}

void EC_StencilGlow::SetStencilGlowEnabled(bool enable)
{
    if (!ViewEnabled())
        return;

    EC_Mesh* mesh = GetMesh();
    if (!mesh)
        return;
    
    Ogre::Entity* entity = mesh->GetEntity();
    if (!entity)
        return;

    if (!outlineSceneNode_ && !outlineEntity_)
        return;

    if (enable && !isEnabled)
    {
        entity->setRenderQueueGroup(STENCIL_GLOW_ENTITY);
        outlineSceneNode_->attachObject(outlineEntity_);
        isEnabled = true;
    }
    else if (!enable && isEnabled)
    {
        entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
        outlineSceneNode_->detachObject(outlineEntity_);
        isEnabled = false;
    }
}

void EC_StencilGlow::OnAttributeUpdated(IAttribute *attribute)
{
    if (attribute == &enabled)
    {
        CreateStencilGlow();
        SetStencilGlowEnabled(enabled.Get());
    }

    else if (attribute == &color)
    {
        if (outlineEntity_ && outlineSceneNode_)
        {
            Ogre::SubEntity *subEnt = outlineEntity_->getSubEntity(0);
            if (subEnt)
            {
                Color newColor = getcolor();
                subEnt->setCustomParameter(COLOR_CUSTOM_PARAM, Ogre::Vector4(newColor.r, newColor.g, newColor.b, newColor.a));
            }
        }
    }
}

void EC_StencilGlow::OnMeshChanged()
{
    CreateStencilGlow();
}

void EC_StencilGlow::OnMeshAboutToBeDestroyed()
{
    DestroyStencilGlow();
}

EC_Mesh* EC_StencilGlow::GetMesh() const
{
    return ParentEntity()->GetComponent<EC_Mesh>().get();
}