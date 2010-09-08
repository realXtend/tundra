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
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();  
    DisableSky();
    
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
    /*RexTypes::Vector3 v = genericSkyParameters.angleAxis;
    Ogre::Quaternion orientation(Ogre::Degree(genericSkyParameters.angle), Ogre::Vector3(v.x, v.y, v.z));*/
 
    ///\todo Get the sky type and other parameters from the config file.
    switch(type_)
    {
    case SKYTYPE_BOX:
    {
        Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyBoxParameters.material);
        skyMaterial->setReceiveShadows(false);
        try
        {
            RexTypes::Vector3 v = skyBoxParameters.angleAxis;
            Ogre::Quaternion orientation(Ogre::Degree(skyBoxParameters.angle), Ogre::Vector3(v.x, v.y, v.z));

            scene_mgr->setSkyBox(show, skyBoxParameters.material, skyBoxParameters.distance, skyBoxParameters.drawFirst, orientation);
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
            RexTypes::Vector3 v = skyDomeParameters.angleAxis;
            Ogre::Quaternion orientation(Ogre::Degree(skyDomeParameters.angle), Ogre::Vector3(v.x, v.y, v.z));

            scene_mgr->setSkyDome(show, skyDomeParameters.material, skyDomeParameters.curvature, skyDomeParameters.tiling,
                skyDomeParameters.distance, skyDomeParameters.drawFirst, orientation, skyDomeParameters.xSegments,
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
            plane.d = skyPlaneParameters.distance;
            plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Z;
            scene_mgr->setSkyPlane(true, plane, skyPlaneParameters.material, skyPlaneParameters.scale, skyPlaneParameters.tiling, true, 
                                    skyPlaneParameters.bow, skyPlaneParameters.xSegments, skyPlaneParameters.ySegments);
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

/*bool EC_OgreSky::SetSkyBox(const std::string& material_name, float distance)
{
    if (renderer_.expired())
        return false;
    RendererPtr renderer = renderer_.lock();  
    DisableSky();
    
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    
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
}*/

StringVector EC_OgreSky::GetMaterialTextureNames()
{
    StringVector texture_names;
    Ogre::MaterialPtr skyMaterial;
    switch(type_)
    {
    case OgreRenderer::SKYTYPE_BOX:
        skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyBoxParameters.material);
        break;
    case OgreRenderer::SKYTYPE_DOME:
        skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyDomeParameters.material);
        break;
    case OgreRenderer::SKYTYPE_PLANE:
        skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyPlaneParameters.material);
        break;
    }

    if (!skyMaterial.isNull())
    {
        Ogre::TextureUnitState *texture_state = skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
        if (texture_state)
            for(uint i = 0; i < texture_state->getNumFrames(); i++)
                texture_names.push_back(texture_state->getFrameTextureName(i));
        //Ogre::String textures = texture_state->getTextureName();
        //texture_names = Ogre::StringConverter::parseStringVector(textures);
    }
    return texture_names;
}

void EC_OgreSky::DisableSky()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();

    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
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
        Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyBoxParameters.material);
        if (!skyMaterial.isNull() && image_count == 6)
        {
            skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&skyBoxImages_[0], false);
            //skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureScale(1, -1);
            CreateSky();
            currentSkyBoxImageCount_ = 0;
        }
        else if(image_count < 6) // If all materials textures dont need to be changed, use code below.
        {
            for(uint i = 0; i < 6; i++)
            {
                if(skyBoxImages_[i] != "")
                    skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setFrameTextureName(Ogre::String(skyBoxImages_[i]), i);
            }
            CreateSky();
            currentSkyBoxImageCount_ = 0;
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

void EC_OgreSky::SetSkyDomeMaterialTexture(const char *texture_name)
{
    type_ = SKYTYPE_DOME;

    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyDomeParameters.material);
    if (!skyMaterial.isNull())
    {
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texture_name);
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        CreateSky();
    }
}

void EC_OgreSky::SetSkyPlaneMaterialTexture(const char *texture_name)
{
    type_ = SKYTYPE_PLANE;

//    const Ogre::String &texture = src_vi->getOgreTexture()->getName();
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyPlaneParameters.material);
    if (!skyMaterial.isNull())
    {
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texture_name);
        skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
        CreateSky();
    }
}

void EC_OgreSky::SetSkyDomeParameters(const SkyDomeParameters &params, bool update_sky)
{
    skyDomeParameters.material = params.material;
    skyDomeParameters.curvature = params.curvature;
    skyDomeParameters.tiling = params.tiling;
    skyDomeParameters.distance = params.distance;
    skyDomeParameters.xSegments = params.xSegments;
    skyDomeParameters.ySegments = params.ySegments;
    skyDomeParameters.ySegmentsKeep = params.ySegmentsKeep;
    skyDomeParameters.drawFirst = params.drawFirst;
    skyDomeParameters.angle = params.angle;
    skyDomeParameters.angleAxis = params.angleAxis;
    if(update_sky)
    {
        type_ = SKYTYPE_DOME;
        CreateSky();
    }
}

void EC_OgreSky::SetSkyPlaneParameters(const SkyPlaneParameters &params, bool update_sky)
{
    skyPlaneParameters.material = params.material;
    skyPlaneParameters.bow = params.bow;
    skyPlaneParameters.scale = params.scale;
    skyPlaneParameters.tiling = params.tiling;
    skyPlaneParameters.distance = params.distance;
    skyPlaneParameters.xSegments = params.xSegments;
    skyPlaneParameters.ySegments = params.ySegments;
    skyPlaneParameters.drawFirst = params.drawFirst;
    if(update_sky)
    {
        type_ = SKYTYPE_PLANE;
        CreateSky();
    }
}

void EC_OgreSky::SetSkyBoxParameters(const SkyBoxParameters &params, bool update_sky)
{
    skyBoxParameters.material = params.material;
    skyBoxParameters.angle = params.angle;
    skyBoxParameters.angleAxis = params.angleAxis;
    skyBoxParameters.distance = params.distance;
    skyBoxParameters.drawFirst = params.drawFirst;
    if(update_sky)
    {
        type_ = SKYTYPE_BOX;
        CreateSky();
    }
}

RexTypes::RexAssetID EC_OgreSky::GetSkyDomeTextureID() const
{
    RexTypes::RexAssetID textureID = "";
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyDomeParameters.material);
    if (!skyMaterial.isNull())
    {
         textureID = skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getFrameTextureName(0);
    }
    return textureID;
}

RexTypes::RexAssetID EC_OgreSky::GetSkyPlaneTextureID() const
{
    RexTypes::RexAssetID textureID = "";
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyPlaneParameters.material);
    if (!skyMaterial.isNull())
    {
         textureID = skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getFrameTextureName(0);
    }
    return textureID;
}

RexTypes::RexAssetID EC_OgreSky::GetSkyBoxTextureID(uint texuture_index) const
{
    if(texuture_index >= 6) texuture_index = 5;
    RexTypes::RexAssetID textureID = "";
    Ogre::MaterialPtr skyMaterial = Ogre::MaterialManager::getSingleton().getByName(skyBoxParameters.material);
    if (!skyMaterial.isNull())
    {
         textureID = skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getFrameTextureName(texuture_index);
    }
    return textureID;
}

SkyBoxParameters EC_OgreSky::GetBoxSkyParameters() const
{
    return skyBoxParameters;
}

SkyDomeParameters EC_OgreSky::GetSkyDomeParameters() const
{
    return skyDomeParameters;
}

SkyPlaneParameters EC_OgreSky::GetSkyPlaneParameters() const
{
    return skyPlaneParameters;
}

bool EC_OgreSky::IsSkyEnabled() const
{
    return skyEnabled_;
}

SkyType EC_OgreSky::GetSkyType() const
{
    return type_;
}

void EC_OgreSky::SetSkyType(SkyType type, bool update_sky)
{
    type_ = type;
    if(update_sky)
        CreateSky();
}

} //namespace OgreRenderer
