/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Billboard.cpp
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
 */

#include "StableHeaders.h"
#include "EC_Billboard.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"

#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QTimer>

#include <Poco/Logger.h>

#define LogError(msg) Poco::Logger::get("EC_Billboard").error("Error: " + std::string(msg));
#define LogInfo(msg) Poco::Logger::get("EC_Billboard").information(msg);

EC_Billboard::EC_Billboard(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    billboardSet_(0),
    billboard_(0),
    materialName_("")
{
}

EC_Billboard::~EC_Billboard()
{
}

void EC_Billboard::SetPosition(const Vector3df& position)
{
    if (IsCreated())
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_Billboard::SetDimensions(float w, float h)
{
    if (IsCreated())
        billboardSet_->setDefaultDimensions(w, h);
}

void EC_Billboard::Show(const std::string &imageName, int timeToShow)
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *scene = renderer->GetSceneManager();
    assert(scene);
    if (!scene)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    OgreRenderer::EC_OgrePlaceable *placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (!placeable)
        return;

    Ogre::SceneNode *node = placeable->GetSceneNode();
    assert(node);
    if (!node)
        return;

    if (imageName.empty())
        return;

    bool succesful = CreateOgreTextureResource(imageName);
    if (!succesful)
        return;

    if (!IsCreated())
    {
        // Billboard not created yet, create it now.
        billboardSet_ = scene->createBillboardSet(renderer->GetUniqueObjectName(), 1);
        assert(billboardSet_);

        materialName_ = std::string("material") + renderer->GetUniqueObjectName(); 
        Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", materialName_);
        OgreRenderer::SetTextureUnitOnMaterial(material, imageName);
        billboardSet_->setMaterialName(materialName_);

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 1.5f));
        assert(billboard_);
        billboardSet_->setDefaultDimensions(0.5f, 0.5f);

        node->attachObject(billboardSet_);
    }
    else
    {
        // Billboard already created Set new texture for the material
        assert(!materialName_.empty());
        if (!materialName_.empty())
        {
            Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
            Ogre::MaterialPtr material = mgr.getByName(materialName_);
            assert(material.get());
            OgreRenderer::SetTextureUnitOnMaterial(material, imageName);
        }
    }

    Show(timeToShow);
}

void EC_Billboard::Show(int timeToShow)
{
    if (IsCreated())
    {
        billboardSet_->setVisible(true);
        if (timeToShow > 0)
            QTimer::singleShot(timeToShow, this, SLOT(Hide()));
    }
}

void EC_Billboard::Hide()
{
    if (IsCreated())
        billboardSet_->setVisible(false);
}

bool EC_Billboard::CreateOgreTextureResource(const std::string &imageName)
{
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(imageName).get());
    if (!tex)
    {
        ///\bug OGRE doesn't seem to add all texture to the resource group although the texture
        ///     exists in folder spesified in the resource.cfg
        std::stringstream ss;
        ss << "Ogre Texture \"" << imageName << "\" not found!";
        LogError(ss.str());

        Ogre::ResourcePtr rp = manager.create(imageName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (!rp.isNull())
        {
            LogInfo("But should be now...");
            return true;
        }

        return false;
    }

    return true;
}

