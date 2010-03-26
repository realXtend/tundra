// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "EnvironmentModule.h"
#include "Sky.h"
#include "SceneManager.h"
#include "CoreTypes.h"
#include "OgreTextureResource.h"
#include "NetworkEvents.h"
#include "ServiceManager.h"
#include "NetworkMessages/NetInMessage.h"

namespace Environment
{
        
Sky::Sky(EnvironmentModule *owner) : owner_(owner), skyEnabled_(false), type_(OgreRenderer::SKYTYPE_BOX), skyBoxImageCount_(0)
{
}

Sky::~Sky()
{
}

bool Sky::HandleRexGM_RexSky(ProtocolUtilities::NetworkEventInboundData* data)
{
    // HACK ON REX MODE, return false if you have problems
    // return false;
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();
    msg.SkipToFirstVariableByName("Parameter");

    // Variable block begins, should have currently (at least) 4 instances.
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    if (instance_count < 4)
    {
        EnvironmentModule::LogWarning("Generic message \"RexSky\" did not contain all the necessary data.");
        return false;
    }

    // 1st instance contains the sky type.
    OgreRenderer::SkyType type = OgreRenderer::SKYTYPE_NONE;
    type = (OgreRenderer::SkyType)boost::lexical_cast<int>(msg.ReadString());

    // 2nd instance contains the texture uuid's
    std::string image_string = msg.ReadString();

    //HACK split() returns vector-struct not a direct vector after verson 6
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1 
    StringVector images_type = Ogre::StringUtil::split(image_string);
    StringVector images = images_type;
#else
    Ogre::vector<Ogre::String>::type images_type = Ogre::StringUtil::split(image_string);
    StringVector images; 
    int size = images_type.size();
    images.resize(size);
    for(int i = 0; i < size; ++i)
      images[i] = images_type[i];
#endif
    //END HACK

    //StringVector images = boost::lexical_cast<StringVector>(images_type);

    // 3rd instance contains the curvature parameter.
    float curvature = boost::lexical_cast<float>(msg.ReadString());

    // 4th instance contains the tiling parameter.
    float tiling = boost::lexical_cast<float>(msg.ReadString());

    UpdateSky(type, images, curvature, tiling);

    return false;
}

void Sky::UpdateSky(const OgreRenderer::SkyType &type, std::vector<std::string> images,
    const float &curvature, const float &tiling)
{
    type_ = type;
    if (type_ == OgreRenderer::SKYTYPE_NONE)
    {
        Scene::EntityPtr sky = GetSkyEntity().lock();
        if (sky)
        {
            OgreRenderer::EC_OgreSky *sky_component = sky->GetComponent<OgreRenderer::EC_OgreSky>().get();
            sky_component->DisableSky();
            return;
        }
    }

    // Suffixes are used to identify different texture positions on the skybox.
    std::map<std::string, int> indexMap;
    std::map<std::string, int>::const_iterator suffixIter;
    indexMap["_fr"] = 0; // front
    indexMap["_bk"] = 1; // back
    indexMap["_lf"] = 2; // left
    indexMap["_rt"] = 3; // right
    indexMap["_up"] = 4; // up
    indexMap["_dn"] = 5; // down

    currentSkyBoxImageCount_ = 0;
    skyBoxImageCount_ = 0;

    size_t max = std::min(images.size(), (size_t)SKYBOX_TEXTURE_COUNT);
    for(size_t n = 0; n < max; ++n)
    {
        std::string image_str = images[n];
        size_t index = n;
        if (image_str.size() < 4)
            break;

        switch(type_)
        {
        case OgreRenderer::SKYTYPE_BOX:
        {
            std::string suffix = image_str.substr(image_str.size() - 3);
            suffixIter = indexMap.find(suffix);
            if (suffixIter == indexMap.end())
                break;
            
            index = suffixIter->second;
            image_str = image_str.substr(0, image_str.size() - 3);

            if (RexTypes::IsNull(image_str))
                break;

            ++skyBoxImageCount_;
            skyBoxTextures_[index] = image_str;
            break;
        }
        case OgreRenderer::SKYTYPE_DOME:
            skyDomeTexture_ = image_str;
            break;
        case OgreRenderer::SKYTYPE_PLANE:
            skyPlaneTexture_ = image_str;
            break;
        }
    }
    
    RequestSkyTextures();

    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (sky) //send changes on curvature and tiling to EC_OgreSky.
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        switch(type)
        {
        case OgreRenderer::SKYTYPE_BOX:
            // Noting to send for sky box so we leave this empty.
            break;
        case OgreRenderer::SKYTYPE_DOME:
            {
                OgreRenderer::SkyDomeParameters param = sky_component->GetSkyDomeParameters();
                if(param.curvature != curvature || param.tiling != tiling)
                {
                    param.curvature = curvature;
                    param.tiling = tiling;
                    sky_component->SetSkyDomeParameters(param);
                }
            break;
            }
        case OgreRenderer::SKYTYPE_PLANE:
            {
                OgreRenderer::SkyPlaneParameters param = sky_component->GetSkyPlaneParameters();
                if(param.tiling != tiling)
                {
                    param.tiling = tiling;
                    sky_component->SetSkyPlaneParameters(param);
                }
            break;
            }
        }
        
        /*OgreRenderer::SkyImageData imageData; //= new OgreRenderer::SkyImageData;
        //imageData.index = index;
        imageData.type = type;
        imageData.curvature = curvature;
        imageData.tiling = tiling;
        sky_component->SetSkyParameters(imageData);*/
    }

    emit SkyTypeChanged();
}

void Sky::CreateDefaultSky(const bool &show)
{
    type_ = OgreRenderer::SKYTYPE_BOX;
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = sky->GetComponent<OgreRenderer::EC_OgreSky>().get();
        assert(sky_component);
        sky_component->CreateSky();
        EnableSky(sky_component->IsSkyEnabled());
        std::vector<std::string> items = sky_component->GetMaterialTextureNames();
        for(uint i = 0; i < 6; i++)
            skyBoxTextures_[i] = items[i];
    }
}

void Sky::DisableSky()
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->DisableSky();
        EnableSky(sky_component->IsSkyEnabled());
    }
}

bool Sky::IsSkyEnabled() const
{
    return skyEnabled_;
}

void Sky::RequestSkyTextures()
{
    boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

    switch(type_)
    {
    case OgreRenderer::SKYTYPE_BOX:
        for(int i = 0; i < SKYBOX_TEXTURE_COUNT; ++i)
            skyBoxTextureRequests_[i] = renderer->RequestResource(skyBoxTextures_[i],
                OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case OgreRenderer::SKYTYPE_DOME:
        skyDomeTextureRequest_ = renderer->RequestResource(skyDomeTexture_,
            OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case OgreRenderer::SKYTYPE_PLANE:
        skyPlaneTextureRequest_ = renderer->RequestResource(skyPlaneTexture_,
            OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case OgreRenderer::SKYTYPE_NONE:
    default:
        break;
    }
}

void Sky::OnTextureReadyEvent(Resource::Events::ResourceReady *tex)
{
    assert(tex);

    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (!sky)
    {
        EnvironmentModule::LogDebug("Could not get SkyEntityPtr!");
        return;
    }

    OgreRenderer::EC_OgreSky *sky_component = sky->GetComponent<OgreRenderer::EC_OgreSky>().get();
    assert(sky_component);

    switch(type_)
    {
    case OgreRenderer::SKYTYPE_BOX:
        for(int i = 0; i < SKYBOX_TEXTURE_COUNT; ++i)
            if (tex->tag_ == skyBoxTextureRequests_[i])
                sky_component->SetSkyBoxMaterialTexture(i, tex->id_.c_str(), skyBoxImageCount_);
        break;
    case OgreRenderer::SKYTYPE_DOME:
        if (tex->tag_ == skyDomeTextureRequest_)
            sky_component->SetSkyDomeMaterialTexture(tex->id_.c_str());
        break;
    case OgreRenderer::SKYTYPE_PLANE:
        if (tex->tag_ == skyPlaneTextureRequest_)
            sky_component->SetSkyPlaneMaterialTexture(tex->id_.c_str());
        break;
    default:
        break;
    }
    EnableSky(sky_component->IsSkyEnabled());
}

void Sky::SetSkyTexture(const RexAssetID &texture_id)
{
    switch(type_)
    {
    case OgreRenderer::SKYTYPE_DOME:
        skyDomeTexture_ = texture_id;
        break;
    case OgreRenderer::SKYTYPE_PLANE:
        skyPlaneTexture_ = texture_id;
        break;
    default:
        EnvironmentModule::LogError("SetSkyTexture can be used only for SkyDome and SkyPlane!");
        break;
    }
}
    
void Sky::SetSkyBoxTextures(const RexAssetID textures[SKYBOX_TEXTURE_COUNT])
{
    if (type_ != OgreRenderer::SKYTYPE_BOX)
    {
        EnvironmentModule::LogError("SetSkyBoxTextures can be used only for SkyBox!");
        return;
    }

    skyBoxImageCount_ = 0;

    for(int i = 0; i < SKYBOX_TEXTURE_COUNT; ++i)
    {
        if(skyBoxTextures_[i] != textures[i] && textures[i] != "")
        {
            skyBoxTextures_[i] = textures[i];
            skyBoxImageCount_++;
        }
    }
}

void Sky::FindCurrentlyActiveSky()
{
    Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        Foundation::ComponentInterfacePtr sky_component = entity.GetComponent(OgreRenderer::EC_OgreSky::NameStatic());
        if (sky_component.get())
            cachedSkyEntity_ = scene->GetEntity(entity.GetId());
    }
}

Scene::EntityWeakPtr Sky::GetSkyEntity()
{
    return cachedSkyEntity_;
}

OgreRenderer::SkyType Sky::GetSkyType() const
{
    return type_;
}

void Sky::EnableSky(bool enabled)
{
    if(skyEnabled_ != enabled)
    {
        skyEnabled_ = enabled;
        emit SkyEnabled(enabled);
    }
}

void Sky::ChangeSkyType(OgreRenderer::SkyType type, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        EnableSky(update_sky);
        sky_component->SetSkyType(type, update_sky);
        type_ = type;
        
        switch(type_)
        {
        case OgreRenderer::SKYTYPE_BOX:
            for(uint i = 0; i < SKYBOX_TEXTURE_COUNT; i++)
                skyBoxTextures_[i] = sky_component->GetSkyBoxTextureID(i);
            break;
        case OgreRenderer::SKYTYPE_PLANE:
            skyPlaneTexture_ = sky_component->GetSkyDomeTextureID();
            break;
        case OgreRenderer::SKYTYPE_DOME:
            skyDomeTexture_ = sky_component->GetSkyDomeTextureID();
            break;
        }

        emit SkyTypeChanged();
    }
}

RexTypes::RexAssetID Sky::GetSkyTextureID(OgreRenderer::SkyType sky_type, int index) const
{
    if(index < 0) index = 0;
    else if(index > SKYBOX_TEXTURE_COUNT - 1) index = SKYBOX_TEXTURE_COUNT - 1;

    if(sky_type == OgreRenderer::SKYTYPE_BOX)
    {
        return skyBoxTextures_[index];
    }
    else if(sky_type == OgreRenderer::SKYTYPE_DOME)
    {
        return skyDomeTexture_;
    }
    else if(sky_type == OgreRenderer::SKYTYPE_PLANE)
    {
        return skyPlaneTexture_;
    }
    return 0;
}

OgreRenderer::SkyDomeParameters Sky::GetSkyDomeParameters()
{
    OgreRenderer::SkyDomeParameters sky_dome_param;
    sky_dome_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_dome_param = sky_component->GetSkyDomeParameters();
    }
    return sky_dome_param;
}

OgreRenderer::SkyPlaneParameters Sky::GetSkyPlaneParameters()
{
    OgreRenderer::SkyPlaneParameters sky_plane_param;
    sky_plane_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_plane_param = sky_component->GetSkyPlaneParameters();
    }
    return sky_plane_param;
}

OgreRenderer::SkyBoxParameters Sky::GetSkyBoxParameters()
{
    OgreRenderer::SkyBoxParameters sky_param;
    sky_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_param = sky_component->GetBoxSkyParameters();
    }
    return sky_param;
}

void Sky::SetSkyDomeParameters(const OgreRenderer::SkyDomeParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyDomeParameters(params, update_sky);
        if(update_sky)
        {
            type_ = OgreRenderer::SKYTYPE_DOME;
            emit SkyTypeChanged();
        }
    }
}

void Sky::SetSkyPlaneParameters(const OgreRenderer::SkyPlaneParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyPlaneParameters(params, update_sky);
        if(update_sky)
        {
            type_ = OgreRenderer::SKYTYPE_PLANE;
            emit SkyTypeChanged();
        }
    }
}

void Sky::SetSkyBoxParameters(const OgreRenderer::SkyBoxParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyBoxParameters(params, update_sky);
        if(update_sky)
        {
            type_ = OgreRenderer::SKYTYPE_BOX;
            emit SkyTypeChanged();
        }
    }
}

} // namespace RexLogic
