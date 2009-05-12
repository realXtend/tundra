// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Terrain.h"

#include <Ogre.h>
#include "../OgreRenderingModule/Renderer.h"

namespace RexLogic
{
    EC_Terrain::EC_Terrain(Foundation::ModuleInterface* module)
    :Foundation::ComponentInterface(module->GetFramework()),
    owner_(module)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService
            <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    }

    EC_Terrain::~EC_Terrain()
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService
            <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
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
    }
}
