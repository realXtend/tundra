/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Highlight.cpp
 *  @brief  EC_Highlight enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_Placeable and EC_Mesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Highlight.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Highlight")

#include "MemoryLeakCheck.h"

EC_Highlight::~EC_Highlight()
{
    // OgreRendering module might be already deleted. If so, the cloned entity is also already deleted.
    // In this case, just set pointer to 0.
    if (!renderer_.expired())
    {
        Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
        if (entityClone_)
            sceneMgr->destroyEntity(entityClone_);
    }
    else
    {
        entityClone_ = 0;
        sceneNode_ = 0;
    }
}

void  EC_Highlight::Show()
{
    if (!entityClone_)
        Create();

    if (!entityClone_)
    {
        LogError("EC_Highlight not initialized properly.");
        return;
    }

    if (entityClone_ && sceneNode_)
        sceneNode_->getAttachedObject(cloneName_)->setVisible(true);
}

void  EC_Highlight::Hide()
{
    if (entityClone_ && sceneNode_)
        sceneNode_->getAttachedObject(cloneName_)->setVisible(false);
}

bool EC_Highlight::IsVisible() const
{
    if (entityClone_ && sceneNode_)
        return sceneNode_->getAttachedObject(cloneName_)->isVisible();

    return false;
}

EC_Highlight::EC_Highlight(IModule *module) :
    IComponent(module->GetFramework()),
    entityClone_(0),
    sceneNode_(0)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
}

void EC_Highlight::Create()
{
    if (renderer_.expired())
        return;

    Ogre::SceneManager *scene = renderer_.lock()->GetSceneManager();
    assert(scene);
    if (!scene)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
    assert(placeable);
    if (!placeable)
        return;

    // Check out if this entity has EC_Mesh or EC_OgreCustomObject.
    Ogre::Entity *originalEntity  = 0;
    if (entity->GetComponent(EC_Mesh::TypeNameStatic()))
    {
        EC_Mesh *ec_mesh= entity->GetComponent<EC_Mesh>().get();
        assert(ec_mesh);

        originalEntity = ec_mesh->GetEntity();
        sceneNode_ = ec_mesh->GetAdjustmentSceneNode();
    }
    else if(entity->GetComponent(EC_OgreCustomObject::TypeNameStatic()))
    {
        EC_OgreCustomObject *ec_custom = entity->GetComponent<EC_OgreCustomObject>().get();
        assert(ec_custom);
        if (!ec_custom->IsCommitted())
        {
            LogError("Mesh entity have not been created for the target primitive. Cannot create EC_Highlight.");
            return;
        }

        originalEntity = ec_custom->GetEntity();
        sceneNode_ = placeable->GetSceneNode();
    }
    else
    {
        LogError("This entity doesn't have either EC_Mesh or EC_OgreCustomObject present. Cannot create EC_Highlight.");
        return;
    }

    if (!originalEntity)
        return; 

    assert(sceneNode_);
    if (!sceneNode_)
        return;

    // Clone the Ogre entity.
    cloneName_ = std::string("entity") + renderer_.lock()->GetUniqueObjectName();
    entityClone_ = originalEntity->clone(cloneName_);

    //This is set so we can exclude the hightlight mesh from a viewport, if so wanted
    entityClone_->setVisibilityFlags(0x1);
    assert(entityClone_);

    // Disable casting of shadows for the clone.
    entityClone_->setCastShadows(false);

    ///\todo If original entity has skeleton, (try to) link it to the clone.
/*
    if (originalEntity->hasSkeleton())
    {
        Ogre::SkeletonInstance *skel = originalEntity->getSkeleton();
        // If sharing a skeleton, force the attachment mesh to use the same skeleton
        // This is theoretically quite a scary operation, for there is possibility for things to go wrong
        Ogre::SkeletonPtr entity_skel = originalEntity->getMesh()->getSkeleton();
        if (entity_skel.isNull())
        {
            LogError("Cannot share skeleton for attachment, not found");
        }
        else
        {
            try
            {
                entityClone_->getMesh()->_notifySkeleton(entity_skel);
            }
            catch (Ogre::Exception &e)
            {
                LogError("Could not set shared skeleton for attachment: " + std::string(e.what()));
            }
        }
    }
*/

    std::string newMatName = std::string("HighlightMaterial") + renderer_.lock()->GetUniqueObjectName();
    try
    {
        Ogre::MaterialPtr highlightMaterial = OgreRenderer::CloneMaterial("Highlight", newMatName);
        entityClone_->setMaterialName(newMatName);
    }
    catch (Ogre::Exception &e)
    {
        LogError("Could not set material \"" + newMatName + "\": " + std::string(e.what()));
        return;
    }

    sceneNode_->attachObject(entityClone_);
}

