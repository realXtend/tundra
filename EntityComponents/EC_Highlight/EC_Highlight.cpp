/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Highlight.cpp
 *  @brief  EC_Highlight enables visual highlighting effect for of scene entity.
 *  @note   The entity must have both EC_OgrePlaceable and EC_OgreMesh components available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Highlight.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "Entity.h"
#include "ModuleLoggingFunctions.h"

#include <Poco/Logger.h>

#include <Ogre.h>

#include "MemoryLeakCheck.h"

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
    OgreRenderer::EC_OgreMesh *ec_mesh= entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    //assert(mesh);
    if (!ec_mesh)
        return;

    Ogre::Entity *ogre_entity = ec_mesh->GetEntity();
    assert(ogre_entity);
    if (!ogre_entity)
        return;

    sceneNode_ = ec_mesh->GetAdjustmentSceneNode();

/*
    StringVector material_names;
    int mat_count = ec_mesh->GetNumMaterials();
    for(i = 0; i < mat_count; ++i)
        material_names.push_back(ec_mesh->GetMaterialName(i));
*/
    //sceneNode_ = placeable->GetSceneNode();

    assert(sceneNode_);
    if (!sceneNode_)
        return;

    cloneName_ = std::string("entity") + renderer_.lock()->GetUniqueObjectName();
    entityClone_ = ogre_entity->clone(cloneName_);
    assert(entityClone_);
/*
    const char *image_name = "rim.dds";
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(image_name).get());
    if (!tex)
    {
        ///\bug OGRE doesn't seem to add all texture to the resource group although the texture
        ///     exists in folder spesified in the resource.cfg
        std::stringstream ss;
        ss << "Ogre Texture \"" << image_name << "\" not found!";
        std::cout << ss.str() << std::endl;

        Ogre::ResourcePtr rp = manager.create(image_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (!rp.isNull())
        {
            std::cout << "But should be now..." << std::endl;
        }
    }
*/
//    std::string origMaterialName = ec_mesh->getmaterialGetMaterialName(0);

    std::string newMatName = std::string("HighlightMaterial") + renderer_.lock()->GetUniqueObjectName();
    try
    {
//        StringVector texture_names;
//        OgreRenderer::GetTextureNamesFromMaterial(oldMaterial, StringVector& texture_names);
        Ogre::MaterialPtr highlightMaterial = OgreRenderer::CloneMaterial("Highlight", newMatName);
//        OgreRenderer::SetTextureUnitOnMaterial(highlightMaterial, "Jack_body_yellow1.jpg");
        entityClone_->setMaterialName(newMatName);
    }
    catch (Ogre::Exception &e)
    {
        LogError("Could not set material \"" + newMatName + "\": " + std::string(e.what()));
        return;
    }

    sceneNode_->attachObject(entityClone_);
//    entityClone_->getParentSceneNode()->setOrientation(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3(1, 0, 0)));
}

