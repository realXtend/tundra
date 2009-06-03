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
        if (renderer)
            Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    }

    EC_Terrain::~EC_Terrain()
    {
        Destroy();
    }

    void EC_Terrain::Destroy()
    {
        assert(owner_);

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
    
    float EC_Terrain::GetPoint(int x, int y)
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= cNumPatchesPerEdge * 16) x = cNumPatchesPerEdge * 16 - 1;
        if (y >= cNumPatchesPerEdge * 16) y = cNumPatchesPerEdge * 16 - 1;
        
        return GetPatch(x / cPatchSize, y / cPatchSize).heightData[(y % cPatchSize) * cPatchSize + (x % cPatchSize)];
    }
    
    Core::Vector3df EC_Terrain::CalculateNormal(int x, int y, int xinside, int yinside)
    {
        int px = x * cPatchSize + xinside;
        int py = y * cPatchSize + yinside;
        
        float x_slope = GetPoint(px-1, py) - GetPoint(px+1, py);
        if ((px <= 0) || (px >= cNumPatchesPerEdge * 16))
            x_slope *= 2;
        float y_slope = GetPoint(px, py-1) - GetPoint(px, py+1);
        if ((py <= 0) || (py >= cNumPatchesPerEdge * 16))
            y_slope *= 2;
        
        Core::Vector3df normal(x_slope, y_slope, 2.0);
        normal.normalize();
        return normal;
    }
}
