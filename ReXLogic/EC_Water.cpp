// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Water.h"

#include <Ogre.h>
#include "../OgreRenderingModule/Renderer.h"

namespace RexLogic
{
    EC_Water::EC_Water(Foundation::ModuleInterface* module)
    :Foundation::ComponentInterface(module->GetFramework()),
    owner_(module), scene_node_(0)
    {
        CreateOgreWaterObject();
    }

    EC_Water::~EC_Water()
    {
        /** \todo Remove the Water data from Ogre nodes.
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer) // Oops! Inconvenient dtor order - can't delete our own stuff since we can't get an instance to the owner.
            return;
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
        if (!sceneMgr) // Oops! Same as above.
            return;

        for(int y = 0; y < cNumPatchesPerEdge; ++y)
            for(int x = 0; x < cNumPatchesPerEdge; ++x)
            {
                Ogre::SceneNode *node = GetPatch(x, y).node;
                if (!node)
                    continue;

                sceneMgr->getRootSceneNode()->removeChild(node);
                sceneMgr->destroyManualObject(dynamic_cast<Ogre::ManualObject*>(node->getAttachedObject(0)));
                node->detachAllObjects();
                sceneMgr->destroySceneNode(node);
            }
        */
    }

    void EC_Water::SetWaterHeight(float height)
    {
        scene_node_->setPosition(scene_node_->getPosition().x, height, scene_node_->getPosition().z);
    }

    float EC_Water::GetWaterHeight() const
    {
        return scene_node_->getPosition().y;
    }

    void EC_Water::CreateOgreWaterObject()
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
        assert(sceneMgr);

        if (!scene_node_)
        {
            const char water_mesh[] = "WaterMesh";
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createPlane(water_mesh, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 5000, 5000, 10, 10, true, 1, 1, 1, Ogre::Vector3::UNIT_X);

            entity_ = sceneMgr->createEntity("WaterEntity", water_mesh);
            entity_->setMaterialName("Ocean");
            entity_->setCastShadows(false);

            scene_node_ = sceneMgr->getRootSceneNode()->createChildSceneNode("WaterNode");
            scene_node_->attachObject(entity_);           
        }
    }
}
