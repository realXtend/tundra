// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Water.h"

#include <Ogre.h>
#include "Renderer.h"

namespace Environment
{
    EC_Water::EC_Water(Foundation::ModuleInterface* module)
    :Foundation::ComponentInterface(module->GetFramework()),
    owner_(module), scene_node_(0)
    {
        CreateOgreWaterObject();
    }

    EC_Water::~EC_Water()
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService
            <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer) // Oops! Inconvenient dtor order - can't delete our own stuff since we can't get an instance to the owner.
            return;
            
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
        if (!sceneMgr) // Oops! Same as above.
            return;
        
        // Destroy the water node and entity.
        scene_node_->detachObject(entity_);
        sceneMgr->getRootSceneNode()->removeAndDestroyChild("WaterNode");
        sceneMgr->destroyEntity(entity_);
    }

    void EC_Water::SetWaterHeight(float height)
    {
        if (scene_node_)
            scene_node_->setPosition(scene_node_->getPosition().x, scene_node_->getPosition().y, height);
    }

    float EC_Water::GetWaterHeight() const
    {
        return scene_node_->getPosition().z;
    }

    void EC_Water::CreateOgreWaterObject()
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService
            <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
            assert(sceneMgr);

            if (!scene_node_)
            {
                const char water_mesh[] = "WaterMesh";
                Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createPlane(water_mesh, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0), 5000, 5000, 10, 10, true, 1, 1, 1, Ogre::Vector3::UNIT_X);

                entity_ = sceneMgr->createEntity("WaterEntity", water_mesh);
                entity_->setMaterialName("Ocean");
                entity_->setCastShadows(false);

                scene_node_ = sceneMgr->getRootSceneNode()->createChildSceneNode("WaterNode");
                scene_node_->attachObject(entity_);
            }
        }
    }
}
