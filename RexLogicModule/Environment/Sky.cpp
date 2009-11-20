// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "RexLogicModule.h"
#include "Environment/Sky.h"
#include "SceneManager.h"
#include "CoreTypes.h"
#include "OgreTextureResource.h"

namespace RexLogic
{

Sky::Sky(RexLogicModule *owner) : owner_(owner), skyEnabled_(false), type_(OgreRenderer::SKYTYPE_BOX), skyBoxImageCount_(0)
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
        RexLogicModule::LogWarning("Generic message \"RexSky\" did not contain all the necessary data.");
        return false;
    }

    // 1st instance contains the sky type.
    OgreRenderer::SkyType type = OgreRenderer::SKYTYPE_NONE;
    type = (OgreRenderer::SkyType)boost::lexical_cast<int>(msg.ReadString());

    // 2nd instance contains the texture uuid's
    std::string image_string = msg.ReadString();

    //HACK split() returns vector-struct not a direct vector after verson 6
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1 
    Core::StringVector images_type = Ogre::StringUtil::split(image_string);
    Core::StringVector images = images_type;
#else
    Ogre::vector<Ogre::String>::type images_type = Ogre::StringUtil::split(image_string);
    Core::StringVector images; 
    int size = images_type.size();
    images.resize(size);
    for(int i = 0; i < size; ++i)
      images[i] = images_type[i];
#endif
    //END HACK

    //Core::StringVector images = boost::lexical_cast<Core::StringVector>(images_type);

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
            OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
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
    skyBoxImages_.clear();
    skyBoxImages_.reserve(skyBoxTextureCount);

    size_t max = std::min(images.size(), (size_t)skyBoxTextureCount);
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

    /*OgreRenderer::SkyImageData *imageData = new OgreRenderer::SkyImageData;
    imageData->index = index;
    imageData->type = type;
    imageData->curvature = curvature;
    imageData->tiling = tiling;*/
}

void Sky::CreateDefaultSky(const bool &show)
{
    type_ = OgreRenderer::SKYTYPE_BOX;
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (sky)
    {
        OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->CreateSky();
    }
}

void Sky::RequestSkyTextures()
{
    boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

    switch(type_)
    {
    case OgreRenderer::SKYTYPE_BOX:
        for(int i = 0; i < skyBoxTextureCount; ++i)
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
        RexLogicModule::LogError("Could not get SkyEntityPtr!");
        return;
    }

    OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
    assert(sky_component);

    switch(type_)
    {
    case OgreRenderer::SKYTYPE_BOX:
        for(int i = 0; i < skyBoxTextureCount; ++i)
            if (tex->tag_ == skyBoxTextureRequests_[i])
                sky_component->SetSkyBoxMaterialTexture(i, tex->id_.c_str(), skyBoxImageCount_);
        break;
    case OgreRenderer::SKYTYPE_DOME:
        if (tex->tag_ == skyDomeTextureRequest_)
            sky_component->SetSkyDomeMaterialTexture(tex->id_.c_str(), 0);
        break;
    case OgreRenderer::SKYTYPE_PLANE:
        if (tex->tag_ == skyPlaneTextureRequest_)
            sky_component->SetSkyPlaneMaterialTexture(tex->id_.c_str());
        break;
    default:
        break;
    }
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
        RexLogicModule::LogError("SetSkyTexture can be used only for SkyDome and SkyPlane!");
        break;
    }
}
    
void Sky::SetSkyBoxTextures(const RexAssetID textures[skyBoxTextureCount])
{
    if (type_ != OgreRenderer::SKYTYPE_DOME)
    {
        RexLogicModule::LogError("SetSkyBoxTextures can be used only for SkyBox!");
        return;
    }

    for(int i = 0; i < skyBoxTextureCount; ++i)
        skyBoxTextures_[i] = textures[i];
}

void Sky::FindCurrentlyActiveSky()
{
    Scene::ScenePtr scene = owner_->GetCurrentActiveScene();
    for(Scene::SceneManager::iterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        Foundation::ComponentInterfacePtr sky_component = entity.GetComponent("EC_OgreSky");
        if (sky_component.get())
            cachedSkyEntity_ = scene->GetEntity(entity.GetId());
    }
}

Scene::EntityWeakPtr Sky::GetSkyEntity()
{
    return cachedSkyEntity_;
}

} // namespace RexLogic
