// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreEnvironment.h"

#include <Ogre.h>

namespace OgreRenderer
{

EC_OgreEnvironment::EC_OgreEnvironment(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    sunlight_(NULL)
//    attached_(false)
{
    InitShadows();
    CreateSunlight();
}

EC_OgreEnvironment::~EC_OgreEnvironment()
{
    if (sunlight_)
    {
        DetachSunlight();
        Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
        sceneManager->destroyLight(sunlight_);
        sunlight_ = NULL;
    }
}

void EC_OgreEnvironment::SetPlaceable(Foundation::ComponentPtr placeable)
{
    DetachSunlight();
    placeable_ = placeable;
    AttachSunlight();
}

void EC_OgreEnvironment::SetBackgoundColor(const Core::Color &color)
{
    renderer_->GetCurrentCamera()->getViewport()->setBackgroundColour(
        Ogre::ColourValue(color.r, color.g, color.b, color.a));
}

Core::Color EC_OgreEnvironment::GetBackgoundColor() const
{
    return Core::Color(renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour().r,
        renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour().g,
        renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour().b,
        renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour().a);
}

void EC_OgreEnvironment::SetAmbientLightColor(const Core::Color &color)
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sceneManager->setAmbientLight(Ogre::ColourValue(color.r, color.g, color.b, color.a));
}

Core::Color EC_OgreEnvironment::GetAmbientLightColor() const
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    
    return Core::Color(sceneManager->getAmbientLight().r,
        sceneManager->getAmbientLight().g,
        sceneManager->getAmbientLight().b,
        sceneManager->getAmbientLight().a);
}

void EC_OgreEnvironment::SetSunColor(const Core::Color& color)
{
    sunlight_->setDiffuseColour(Ogre::ColourValue(color.r, color.g, color.b, color.a));
}

void EC_OgreEnvironment::SetSunAttenuation(float range, float constant, float linear, float quad)
{
    sunlight_->setAttenuation(range, constant, linear, quad);
}

void EC_OgreEnvironment::SetSunDirection(const Core::Vector3df& direction)
{
    sunlight_->setDirection(Ogre::Vector3(direction.x, direction.y, direction.z));
}
    
void EC_OgreEnvironment::SetSunCastShadows(const bool &enabled)
{
    sunlight_->setCastShadows(enabled);
}

void EC_OgreEnvironment::CreateSunlight()
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sunlight_ = sceneManager->createLight(renderer_->GetUniqueObjectName());
    sunlight_->setType(Ogre::Light::LT_DIRECTIONAL);
    ///\todo Read parameters from config file?
    sunlight_->setDiffuseColour(0.93f, 1, 0.13f);
    sunlight_->setPosition(0, 255, 100);
    sunlight_->setDirection(125, 125, 5);
//    sunlight_->setAttenuation();
    sunlight_->setCastShadows(true);   
}

void EC_OgreEnvironment::AttachSunlight()
{
    if ((placeable_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(sunlight_);
    }
}

void EC_OgreEnvironment::DetachSunlight()
{
    if ((placeable_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(sunlight_);
    }
}

void EC_OgreEnvironment::InitShadows()
{
    /*
    float shadowFarDist = 15;
    unsigned long shadowTextureSize = 512;
    float shadowFadeStart = 0.7;
    float shadowFadeEnd = 0.9;
    float shadowDirLightTextureOffset = 0.6;
    int shadowTextureCount = 1;
    Ogre::ColourValue shadowColor(0.6f, 0.6f, 0.6f);
    bool useObjectShadows = false;
    int shadowMethod = 2; //enum, ShadowsNone
    
    // This is the default material to use for shadow buffer rendering pass, overridable in script.
    // Note that we use the same single material (vertex program) for each object, so we're relying on
    // that we use Ogre software skinning. Hardware skinning would require us to do different vertex programs
    // for skinned/nonskinned geometry.
    std::string ogreShadowCasterMaterial = "rex/ShadowCaster"; 
    
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sceneManager->setShadowColour(shadowColor);
    sceneManager->setShadowFarDistance(shadowFarDist);

    sceneManager->setShadowTextureSize(shadowTextureSize);
    sceneManager->setShadowDirLightTextureOffset(shadowDirLightTextureOffset);
    sceneManager->setShadowTextureFadeStart(shadowFadeStart);
    sceneManager->setShadowTextureFadeEnd(shadowFadeEnd);
    sceneManager->setShadowTextureCount(shadowTextureCount);

    // Set shadow mode to texture rather than stencil, so object geometry has less of an impact
    //if (shadowMethod == ShadowsPCF)
    {
	    sceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT16_R);
	    sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
	    sceneManager->setShadowTextureCasterMaterial(ogreShadowCasterMaterial.c_str());
	    sceneManager->setShadowTextureSelfShadow(true);
    }
    */
}

}
