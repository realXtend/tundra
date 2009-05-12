// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "RexLogicModule.h"
#include "Sky.h"
#include "EC_OgreSky.h"

#include "../OgreRenderingModule/OgreTextureResource.h"

#include <Ogre.h>

namespace RexLogic
{

Sky::Sky(RexLogicModule *owner) : owner_(owner), skyEnabled_(false)
{
}

Sky::~Sky()
{
}

void Sky::CreateDefaultSky(bool show)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    OgreRenderer::EC_OgreSky *sky_component = checked_static_cast<OgreRenderer::EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
    assert(sky_component);
    
    ///\todo change to CreateDefaultSky()
    sky_component->CreateDefaultSkybox();

}

void Sky::RequestSkyboxTextures()
{
    boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

    for(int i = 0; i < skyboxTextureCount; ++i)
        skyboxTextureRequests_[i] = renderer->RequestResource(skyboxTextures_[i].ToString(), OgreRenderer::OgreTextureResource::GetTypeStatic());
}

void Sky::OnTextureReadyEvent(Resource::Events::ResourceReady *tex)
{
    assert(tex);

    /*for(int i = 0; i < num_terrain_textures; ++i)
        if (tex->tag_ == terrain_texture_requests_[i])
            SetSkyMaterialTexture(i, tex->id_.c_str());*/
}

bool Sky::HandleRexGM_RexSky(OpenSimProtocol::NetworkEventInboundData* data)
{
    NetInMessage &msg = *data->message;
    msg.ResetReading();
    msg.SkipToFirstVariableByName("Parameter");
    
    // Variable block begins
    size_t instance_count = msg.ReadCurrentBlockInstanceCount();
    if (instance_count < 4)
    {
        RexLogicModule::LogError("Generic message \"RexSky\" did not contain all the necessary data.");
        return false;
    }
    
    // First instance contains the sky type.
    int type = OgreRenderer::SKYTYPE_NONE;
    type = msg.ReadU8();
    switch(type)
    {
    case 0:
        type = OgreRenderer::SKYTYPE_NONE;
        break;
    case 1:
        type = OgreRenderer::SKYTYPE_BOX;
        break;
    case 2:
        type = OgreRenderer::SKYTYPE_DOME;
        break;
    //\todo Support for skyplane
    }
    
    std::string image_string = msg.ReadString();
    Ogre::StringVector images = Ogre::StringUtil::split(image_string);
    float curvature = msg.ReadF32();
    float tiling = msg.ReadF32();
    
//    UpdateSky(type, images, curvature, tiling);
    
    return false;
}

void Sky::FindCurrentlyActiveSky()
{
    Scene::ScenePtr scene = owner_->GetCurrentActiveScene();
    for(Scene::SceneManagerInterface::EntityIterator iter = scene->begin();
        iter != scene->end(); ++iter)
    {
        Scene::EntityInterface &entity = *iter;
        Foundation::ComponentInterfacePtr sky_component = entity.GetComponent("EC_OgreSky");
        if (sky_component.get())
        {
            cachedSkyEntity_ = scene->GetEntity(entity.GetId());
        }
    }
}

Scene::EntityWeakPtr Sky::GetSkyEntity()
{
    return cachedSkyEntity_;
}

/*
void LLOgreRenderer::updateSky(RexSkyType type, const std::vector<std::string> &images, F32 curvature, F32 tiling)
{
   if (type == REXSKY_NONE)
   {
      mCurrentSkyType = type;
      createSky(false);
   } else
   {
      std::map<std::string, int> indexMap;
      std::map<std::string, int>::const_iterator suffixIter;
      indexMap["_fr"] = 0;
      indexMap["_bk"] = 1;
      indexMap["_lf"] = 2;
      indexMap["_rt"] = 3;
      indexMap["_up"] = 4;
      indexMap["_dn"] = 5;

      mCurrentSkyBoxImageCount = 0;
      mSkyBoxImageCount = 0;
      mSkyboxImages.clear();
      mSkyboxImages.reserve(6);

      size_t max = std::min(images.size(), (size_t)6);
      size_t n;
      for (n=0 ; n<max ; ++n)
      {
         std::string imageStr = images[n];
         int index = (int)n;
         if (imageStr.size() > 3)
         {
            std::string suffix = imageStr.substr(imageStr.size() - 3);
            suffixIter = indexMap.find(suffix);
            if (suffixIter != indexMap.end())
            {
               index = suffixIter->second;
               imageStr = imageStr.substr(0, imageStr.size() - 3);
            }
         }
         LLUUID imageId(imageStr);

         if (imageId.notNull())
         {
            mSkyBoxImageCount++;

            LLViewerImage *image = gImageList.getImage(imageId, FALSE, TRUE, 0, 0, gAgent.getRegionHost());
            SkyImageData *imageData = new SkyImageData;
            imageData->mIndex = index;
            imageData->mType = type;
            imageData->mCurvature = curvature;
            imageData->mTiling = tiling;
            image->setLoadedCallback(onSkyTextureLoaded, 0, FALSE, (void*)imageData);
         }
      }
   }
}*/

} // namespace RexLogic
