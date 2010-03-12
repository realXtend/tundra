/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Highlight.cpp
 *  @brief  EC_Highlight enables visual highlighting effect for of scene entity.
 *  @note   The entity must have both EC_OgrePlaceable and EC_OgreMesh components available in advance.
 */

#include "StableHeaders.h"
#include "EC_Highlight.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "Entity.h"
#include "ModuleLoggingFunctions.h"

#include <Poco/Logger.h>

#include <Ogre.h>

#define LogInfo(msg) Poco::Logger::get("EC_Highlight").error("Error: " + msg);
#define LogError(msg) Poco::Logger::get("EC_Highlight").information(msg);

EC_Highlight::EC_Highlight(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    entityClone_(0),
    sceneNode_(0)
{
    renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
}

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

//    assert(entityClone_);
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

    OgreRenderer::EC_OgrePlaceable *placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    assert(placeable);
    if (!placeable)
        return;

    // Prims have no EC_OgreMesh necessarily so no assertion here.
    OgreRenderer::EC_OgreMesh *mesh= entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    //assert(mesh);
    if (!mesh)
        return;

    Ogre::Entity *ogre_entity = mesh->GetEntity();
    assert(ogre_entity);
    if (!ogre_entity)
        return;

    sceneNode_ = mesh->GetAdjustmentSceneNode();
    //sceneNode_ = placeable->GetSceneNode();
    assert(sceneNode_);
    if (!sceneNode_)
        return;

    cloneName_ = std::string("entity") + renderer_.lock()->GetUniqueObjectName();
    entityClone_ = ogre_entity->clone(cloneName_);
    assert(entityClone_);

    const std::string &material_name = "Highlight";
    try
    {
        entityClone_->setMaterialName(material_name);
    }
    catch (Ogre::Exception &e)
    {
        LogError("Could not set material \"" + material_name + "\": " + std::string(e.what()));
        return;
    }

    sceneNode_->attachObject(entityClone_);
//    entityClone_->getParentSceneNode()->setOrientation(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3(1, 0, 0)));
}

