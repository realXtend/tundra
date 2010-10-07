/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Clone.cpp
 *  @brief  EC_Clone creates an OGRE clone entity from the the original entity.
 *          This component can be used e.g. when visualizing object duplication in the world.
 *  @note   The entity must have EC_Placeable and EC_Mesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Clone.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Clone")

#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_Clone::EC_Clone(IModule *module) :
    IComponent(module->GetFramework()),
    entityClone_(0),
    sceneNode_(0)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
}

EC_Clone::~EC_Clone()
{
    // OgreRendering module might be already deleted. If so, the cloned entity and scene node is 
    // also already deleted. In this case, just set pointers to 0.
    if (!renderer_.expired())
    {
        Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
        if (entityClone_)
        {
            sceneMgr->destroyEntity(entityClone_);
            entityClone_ = 0;
        }
        if (sceneNode_)
        {
            sceneMgr->destroySceneNode(sceneNode_);
            sceneNode_ = 0;
        }
    }
    else
    {
        entityClone_ = 0;
        sceneNode_ = 0;
    }
}

void EC_Clone::Show()
{
    if (!entityClone_)
        Create();

    if (!entityClone_)
    {
        LogError("EC_Clone not initialized properly.");
        return;
    }

    if (entityClone_ && sceneNode_)
        sceneNode_->getAttachedObject(cloneName_)->setVisible(true);
}

void  EC_Clone::Hide()
{
    if (entityClone_ && sceneNode_)
        sceneNode_->getAttachedObject(cloneName_)->setVisible(false);
}

bool EC_Clone::IsVisible() const
{
    if (entityClone_ && sceneNode_)
        return sceneNode_->getAttachedObject(cloneName_)->isVisible();

    return false;
}

QVector3D EC_Clone::GetPosition() const
{
    if (entityClone_ && sceneNode_)
        return QVector3D(sceneNode_->getPosition().x, sceneNode_->getPosition().y, sceneNode_->getPosition().z);

    return QVector3D();
}

void EC_Clone::Create()
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

    Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
    assert(sceneMgr);
    if (!sceneMgr)
        return;

    Ogre::Entity *originalEntity  = 0;
    Ogre::SceneNode *originalNode = 0;

    // Check out if this entity has EC_Mesh or EC_OgreCustomObject.
    if (entity->GetComponent(EC_Mesh::TypeNameStatic()))
    {
        EC_Mesh *ec_mesh= entity->GetComponent<EC_Mesh>().get();
        assert(ec_mesh);

        originalEntity = ec_mesh->GetEntity();
        //originalNode = placeable->GetSceneNode();
        originalNode = ec_mesh->GetAdjustmentSceneNode();

        sceneNode_ = sceneMgr->createSceneNode();
        sceneNode_->setPosition(originalNode->getPosition());
        originalNode->addChild(sceneNode_);
    }
    else if(entity->GetComponent(EC_OgreCustomObject::TypeNameStatic()))
    {
        EC_OgreCustomObject *ec_custom = entity->GetComponent<EC_OgreCustomObject>().get();
        assert(ec_custom);
        if (!ec_custom->IsCommitted())
        {
            LogError("Mesh entity have not been created for the target primitive. Cannot create EC_Clone.");
            return;
        }

        originalEntity = ec_custom->GetEntity();
        originalNode = placeable->GetSceneNode();

        sceneNode_ = sceneMgr->createSceneNode();
        sceneNode_->setPosition(originalNode->getPosition());
        originalNode->addChild(sceneNode_);
    }
    else
    {
        LogError("This entity doesn't have either EC_Mesh or EC_OgreCustomObject present. Cannot create EC_Clone.");
        return;
    }

    assert(originalEntity);
    if (!originalEntity)
        return;

    assert(sceneNode_);
    if (!sceneNode_)
        return;

    // Clone the Ogre entity.
    cloneName_ = std::string("entity") + renderer_.lock()->GetUniqueObjectName();
    entityClone_ = originalEntity->clone(cloneName_);
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

    const std::string &material_name = "Clone";
    try
    {
        entityClone_->setMaterialName(material_name);
    }
    catch (Ogre::Exception &e)
    {
        LogError("Could not set material \"" + std::string(material_name) + "\": " + std::string(e.what()));
        return;
    }

    sceneNode_->attachObject(entityClone_);
}

