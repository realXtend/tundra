// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgreSky.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreSky::EC_OgreSky(Foundation::ModuleInterface* module) :
        renderer_(static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        sky_enabled_(false)
    {
    }
    
    EC_OgreSky::~EC_OgreSky()
    {
        DisableSky();
    }
    
    bool EC_OgreSky::SetSkyBox(const std::string& material_name, Core::Real distance)
    {
        DisableSky();
        
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        try
        {
            scene_mgr->setSkyBox(true, material_name, distance);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set skybox: " + std::string(e.what()));
            return false;
        }
        
        sky_enabled_ = true;
        return true;
    }
    
    void EC_OgreSky::DisableSky()
    {
        if (sky_enabled_)
        {
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            scene_mgr->setSkyBox(false, "");
            
            sky_enabled_ = false;
        }
    }
}