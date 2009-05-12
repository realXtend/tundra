// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgreSky.h"
#include "ConversionUtils.h"

#include <Ogre.h>

namespace OgreRenderer
{

EC_OgreSky::EC_OgreSky(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    owner_(module),
    skyEnabled_(false),
    type_(SKYTYPE_BOX)
{
    skyboxImages_.resize(6);
}

EC_OgreSky::~EC_OgreSky()
{
    DisableSky();
}

void EC_OgreSky::CreateDefaultSkybox()
{
    DisableSky();
    
    Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
    
    try
    {
        scene_mgr->setSkyBox(true, genericSkyParameters.material, genericSkyParameters.distance);
        skyEnabled_ = true;
    }
    catch (Ogre::Exception& e)
    {
        OgreRenderingModule::LogError("Could not set skybox: " + std::string(e.what()));
    }
}

void EC_OgreSky::CreateDefaultSky(bool show)
{
    DisableSky();
    Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();

//    Ogre::Quaternion orientation(Ogre::Degree(genericSkyParameters.angle), Core::OpenSimToOgreCoordinateAxes(genericSkyParameters.angleAxis));
    Ogre::Quaternion orientation(0,0,0);

    ///\todo Read the sky type from the config file.
    switch(type_)
    {
    case SKYTYPE_BOX:
    {
        Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
        skyMaterial->setReceiveShadows(false);
        try
        {
            scene_mgr->setSkyBox(show, genericSkyParameters.material, genericSkyParameters.distance, genericSkyParameters.drawFirst, orientation);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set skybox: " + std::string(e.what()));
            return;
        }                
        skyEnabled_ = true;
        break;
    }
    case SKYTYPE_DOME:
        try
        {
            scene_mgr->setSkyBox(false, "");
            scene_mgr->setSkyDome(show, genericSkyParameters.material, skydomeParameters.curvature, skydomeParameters.tiling,
                genericSkyParameters.distance, genericSkyParameters.drawFirst, orientation, skydomeParameters.xSegments,
                skydomeParameters.ySegments, skydomeParameters.ySegmentsKeep);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set skydome: " + std::string(e.what()));
            return;
        }                
        
        skyEnabled_ = true;
        break;
    ///\todo case SKYTYPE_PLANE:
    case SKYTYPE_NONE:
    default:
        skyEnabled_ = false;
        break;
    }
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
    
    skyEnabled_ = true;
    return true;
}

void EC_OgreSky::DisableSky()
{
    Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
    scene_mgr->setSkyBox(false, "");
    scene_mgr->setSkyDome(false, "");
    scene_mgr->setSkyPlane(false, Ogre::Plane(0, 0, 0, 0), "");
    skyEnabled_ = false;
}

/*void EC_OgreSky::SetSkyBoxMaterialName()
{
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
    if (skyMaterial.isNull() == false)
    {
        skyMaterial->setReceiveShadows(false);
        if (skyboxImages_.size() == 6)
            skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&skyboxImages_[0], false);
    }
}*/

} //namespace OgreRenderer
