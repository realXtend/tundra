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
    type_(SKYTYPE_BOX),
    currentSkyBoxImageCount_(0)
{
    skyBoxImages_.resize(6);
    ///\todo GetSkyConfig()
}

EC_OgreSky::~EC_OgreSky()
{
    DisableSky();
}

/*void EC_OgreSky::CreateDefaultSkybox()
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
}*/

void EC_OgreSky::CreateSky(bool show)
{
    DisableSky();
    
    Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
    
    RexTypes::Vector3 v = genericSkyParameters.angleAxis;
    Ogre::Quaternion orientation(Ogre::Degree(genericSkyParameters.angle), Ogre::Vector3(v.x, v.y, v.z));
 
    ///\todo Get the sky type and other parameters from the config file.
    switch(type_)
    {
    case SKYTYPE_BOX:
    {
        Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
        skyMaterial->setReceiveShadows(false);
        try
        {
//            if (skyboxImages_.size() == 6)
                scene_mgr->setSkyBox(show, genericSkyParameters.material, genericSkyParameters.distance, genericSkyParameters.drawFirst, orientation);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set SkyBox: " + std::string(e.what()));
            return;
        }                
        
        skyEnabled_ = true;
        break;
    }
    case SKYTYPE_DOME:
        try
        {
            scene_mgr->setSkyDome(show, genericSkyParameters.material, skyDomeParameters.curvature, skyDomeParameters.tiling,
                genericSkyParameters.distance, genericSkyParameters.drawFirst, orientation, skyDomeParameters.xSegments,
                skyDomeParameters.ySegments, skyDomeParameters.ySegmentsKeep);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set SkyDome: " + std::string(e.what()));
            return;
        }

        skyEnabled_ = true;
        break;
    case SKYTYPE_PLANE:
        try
        {
            ///\todo
            Ogre::Plane plane;
            plane.d = 1000;
            plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;
            scene_mgr->setSkyPlane(true, plane, genericSkyParameters.material, 1500, 40, true, 1.5f, 150, 150);            
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set SkyPlane: " + std::string(e.what()));
            return;
        }
        
        skyEnabled_ = true;
        break;
    case SKYTYPE_NONE:
    default:
        skyEnabled_ = false;
        break;
    }
}

bool EC_OgreSky::SetSkyBox(const std::string& material_name, Real distance)
{
    DisableSky();
    
    Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
    
    try
    {
        scene_mgr->setSkyBox(true, material_name, distance);
    }
    catch (Ogre::Exception& e)
    {
        OgreRenderingModule::LogError("Could not set SkyBox: " + std::string(e.what()));
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
    scene_mgr->setSkyPlane(false, Ogre::Plane(), "");
    skyEnabled_ = false;
}

void EC_OgreSky::GetSkyConfig()
{
    ///\todo Read different parameters from the config file.
}

void EC_OgreSky::SetSkyBoxMaterialTexture(int index, const char *texture_name, size_t image_count/*const SkyImageData *imageData*/)
{
    type_ = SKYTYPE_BOX;
    //genericSkyParameters.disance = distance;
    // The SkyBox textures are upside down, because there is no easy way to flip cubemap textures vertically.
//    skyBoxImages_.resize(6);
    skyBoxImages_[index] = texture_name;
    
    ++currentSkyBoxImageCount_;
    if (currentSkyBoxImageCount_ == image_count)
    {
        Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
        if (!skyMaterial.isNull())
        {
            skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&skyBoxImages_[0], false);
            //skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureScale(1, -1);
            CreateSky();
        }
        
        ///\todo
        /*
        // Lets create scaled down cubemap texture for water reflections.
        // We apparently need to create a separate cubemap texture for the environment map as
        // there's no way to use the existing ones. It's also good idea to scale the cubemap down since
        // it doesn't need to be very accurate.

        // For ease, we create scaled down versions of the skybox textures and then copy the new textures into
        // cubemap faces and afterwards remove the new scaled down textures.
        std::vector<Ogre::TexturePtr> scaledTextures;
        scaledTextures.reserve(skyBoxImageCount_);
        for (size_t n = 0; n < skyBoxImages_.size(); ++n)
        {
            Ogre::String defaultTextureName = skyBoxImages_[n];
            if (!defaultTextureName.empty())
            {
                Ogre::TexturePtr defaultTexture = Ogre::TextureManager::getSingleton().getByName(defaultTextureName);
                if (!defaultTexture.isNull())
                {
                    Ogre::uint width = std::max(defaultTexture->getWidth() / 2, (Ogre::uint)2);
                    Ogre::uint height = std::max(defaultTexture->getHeight() / 2, (Ogre::uint)2);

                    Ogre::TexturePtr newTexture = Ogre::TextureManager::getSingleton().createManual("scaled" + Ogre::StringConverter::toString(n),
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, Ogre::MIP_UNLIMITED, defaultTexture->getBuffer()->getFormat());
                    defaultTexture->copyToTexture(newTexture);
                    scaledTextures.push_back(newTexture);
                }
            }
        }

        Ogre::TexturePtr cubeTex = (Ogre::TexturePtr)Ogre::TextureManager::getSingleton().getByName("RexSkyboxCubic");
        if (cubeTex.isNull())
        {
            Ogre::TexturePtr tex = scaledTextures[0];
            cubeTex = Ogre::TextureManager::getSingleton().createManual("RexSkyboxCubic",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_CUBE_MAP,
                tex->getWidth(), tex->getHeight(), tex->getDepth(), tex->getNumMipmaps(), tex->getBuffer()->getFormat());
        }

        const int side[] = {3, 2, 4, 5, 0, 1};
        for (int i = 0; i < skyBoxImageCount_; ++i)
        {
            size_t index = side[i];
            if (index < scaledTextures.size())
            {
                Ogre::TexturePtr face = scaledTextures[index];
                Ogre::HardwarePixelBufferSharedPtr buffer = cubeTex->getBuffer(i);
                buffer->blit(face->getBuffer());
            }
        }

        for(n = 0; n < scaledTextures.size(); ++n)
            Ogre::TextureManager::getSingleton().remove(static_cast<Ogre::ResourcePtr>(scaledTextures[n]));
        
        ///\todo Update water reflection
        Ogre::MaterialPtr waterMaterial = Ogre::MaterialManager::getSingleton().getByName(mWaterMaterial);
        if (!waterMaterial.isNull())
        {
            llinfos << "Updating water material..." << llendl;
            Ogre::TextureUnitState* state = waterMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(1);
            state->setCubicTextureName("RexSkyboxCubic", true);
            state->setTextureCoordSet(1);
        }*/
    }
}

void EC_OgreSky::SetSkyDomeMaterialTexture(const char *texture_name, const SkyImageData *parameters)
{
    type_ = SKYTYPE_DOME;
///\todo pass the parameters
//    skyDomeParameters.curvature = parameters->curvature;
//    skyDomeParameters.tiling = parameters->tiling;

    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
    if (!skyMaterial.isNull())
    {
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texture_name);
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        CreateSky();
    }
}

void EC_OgreSky::SetSkyPlaneMaterialTexture(const char *texture_name)
{
///\todo
/*
    type_ = SKYTYPE_PLANE;
    skyDomeParameters.curvature = imageData->curvature;
    skyDomeParameters.tiling = imageData->tiling;

//    const Ogre::String &texture = src_vi->getOgreTexture()->getName();
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(genericSkyParameters.material);
    if (!skyMaterial.isNull())
    {
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texture_name);
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        CreateSky();
    }
*/
}

} //namespace OgreRenderer
