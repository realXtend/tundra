// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Sky.h"

#include "Foundation.h"
#include "OgreRenderingModule.h"

#include "CoreTypes.h"
#include "OgreTextureResource.h"
#include "NetworkEvents.h"
#include "ServiceManager.h"
#include "NetworkMessages/NetInMessage.h"


namespace Environment
{
        
Sky::Sky(EnvironmentModule *owner) : owner_(owner), skyEnabled_(false), type_(SKYTYPE_BOX), skyBoxImageCount_(0)
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
    SkyType type = SKYTYPE_NONE;
    type = (SkyType)boost::lexical_cast<int>(msg.ReadString());

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

void Sky::UpdateSky(const SkyType &type, std::vector<std::string> images,
    const float &curvature, const float &tiling)
{
    type_ = type;
    if (type_ == SKYTYPE_NONE)
    {
        DisableSky();       
        emit SkyTypeChanged();
        return;
        /*       
        Scene::EntityPtr sky = GetSkyEntity().lock();
        if (sky)
        {
            EC_OgreSky *sky_component = sky->GetComponent<EC_OgreSky>().get();
            sky_component->DisableSky();
            return;
        }
        */
    }


    /*


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
        case SKYTYPE_BOX:
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
        case SKYTYPE_DOME:
            skyDomeTexture_ = image_str;
            break;
        case SKYTYPE_PLANE:
            skyPlaneTexture_ = image_str;
            break;
        }
    }
    */

    boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

    switch (type_ )
    {
        case SKYTYPE_BOX:
        {
            // Assure that there exist skybox
            if ( !ExistSky<EC_SkyBox>() )
                CreateSky<EC_SkyBox>();
            
            // Request textures. 
            for ( int i = 0;  i < images.size() && i <= 6; ++i)
            {
                int tag = renderer->RequestResource(images[i], OgreRenderer::OgreTextureResource::GetTypeStatic());     
                QString str(images[i].c_str());
                
                if ( str.contains("_fr") )
                {
                    requestMap_[tag] = 0;
                }
                else if ( str.contains("_bk") )
                {
                    requestMap_[tag] = 1;   
                }
                else if ( str.contains("_lf") )
                {
                    requestMap_[tag] = 2;   
                }
                else if ( str.contains("_rt") )
                {
                    requestMap_[tag] = 3;   
                }
                else if ( str.contains("_up") )
                {
                    requestMap_[tag] = 4;   
                }
                else if ( str.contains("_dn") )
                {
                    requestMap_[tag] = 5;   
                }
                
                lstRequestTags_.append(tag);
           
            }
            break;
        }
        case SKYTYPE_DOME:
        {
            //Assure that there exist skydome
            
            // Request textures
            lstRequestTags_.append(renderer->RequestResource(images[0], OgreRenderer::OgreTextureResource::GetTypeStatic()));
            break;
        }
        case SKYTYPE_PLANE:
        {
            //Assure that there exist skyplane

            if ( !ExistSky<EC_SkyPlane>())
                CreateSky<EC_SkyPlane>();

            EC_SkyPlane* sky = GetEnviromentSky<EC_SkyPlane >();
            float component_tiling = sky->tilingAttr.Get();
            if (component_tiling != tiling )
            {
                sky->tilingAttr.Set(tiling, AttributeChange::LocalOnly);
            }

            lstRequestTags_.append(renderer->RequestResource(images[0], OgreRenderer::OgreTextureResource::GetTypeStatic()));
            break;
        }
        
        default:
            break;

    }

    //RequestSkyTextures();

    /*
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (sky) //send changes on curvature and tiling to EC_OgreSky.
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        switch(type)
        {
        case SKYTYPE_BOX:
            // Noting to send for sky box so we leave this empty.
            break;
        case SKYTYPE_DOME:
            {
                SkyDomeParameters param = sky_component->GetSkyDomeParameters();
                if(param.curvature != curvature || param.tiling != tiling)
                {
                    param.curvature = curvature;
                    param.tiling = tiling;
                    sky_component->SetSkyDomeParameters(param);
                }
            break;
            }
        case SKYTYPE_PLANE:
            {
                SkyPlaneParameters param = sky_component->GetSkyPlaneParameters();
                if(param.tiling != tiling)
                {
                    param.tiling = tiling;
                    sky_component->SetSkyPlaneParameters(param);
                }
            break;
            }
        }
        */

        /*OgreRenderer::SkyImageData imageData; //= new OgreRenderer::SkyImageData;
        //imageData.index = index;
        imageData.type = type;
        imageData.curvature = curvature;
        imageData.tiling = tiling;
        sky_component->SetSkyParameters(imageData);*/
    

    emit SkyTypeChanged();
}

void Sky::CreateDefaultSky(const bool &show)
{
    type_ = SKYTYPE_BOX;
    // Exist sky? 
    if ( !ExistSky<EC_SkyBox>() )
        CreateSky<EC_SkyBox>();

    EnableSky(true);
    /*
    type_ = OgreRenderer::SKYTYPE_BOX;
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (sky)
    {
        EC_OgreSky *sky_component = sky->GetComponent<EC_OgreSky>().get();
        assert(sky_component);
        sky_component->CreateSky();
        EnableSky(sky_component->IsSkyEnabled());
        std::vector<std::string> items = sky_component->GetMaterialTextureNames();
        for(uint i = 0; i < 6; i++)
            skyBoxTextures_[i] = items[i];
    }
    */
}

void Sky::DisableSky()
{
    RemoveSky<EC_SkyBox>();
    RemoveSky<EC_SkyPlane>();
    
    
   //switch (type_)
   //{    
   //    case OgreRenderer::SKYTYPE_BOX:
   //        {
   //            RemoveSky<EC_SkyBox>();
   //            
   //             /*
   //             EC_SkyBox* sky = GetEnviromentSky<EC_SkyBox >();
   //             if ( sky != 0)
   //             {
   //                 sky->DisableSky();
   //                 EnableSky(false);
   //             }
   //             */
   //             break;
   //        }
   //    case OgreRenderer::SKYTYPE_PLANE:
   //        {
   //             RemoveSky<EC_SkyPlane>();
   //             /*
   //             EC_SkyPlane* sky = GetEnviromentSky<EC_SkyPlane >();
   //             if ( sky != 0)
   //             {
   //                 sky->DisableSky();
   //                 EnableSky(false);
   //             }
   //             */
   //             break;       
   //        }
   //    case OgreRenderer::SKYTYPE_DOME:
   //        {
   //             break;
   //        }
   //    default:
   //        break;

   //}




    /*
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->DisableSky();
        EnableSky(sky_component->IsSkyEnabled());
    }
    */
}

void Sky::Update()
{
   switch (type_)
   {    
       case SKYTYPE_BOX:
           {
                EC_SkyBox* sky = GetEnviromentSky<EC_SkyBox >();
                if (sky == 0)
                {
                    CreateSky<EC_SkyBox >();
                    EC_SkyBox* sky = GetEnviromentSky<EC_SkyBox >();
                 //   sky->ComponentChanged(AttributeChange::Replicate);
                }
                break;
           }
       case SKYTYPE_PLANE:
           {
                EC_SkyPlane* sky = GetEnviromentSky<EC_SkyPlane >();
                if (sky == 0)
                {
                    CreateSky<EC_SkyPlane>();
                    EC_SkyPlane* sky = GetEnviromentSky<EC_SkyPlane >();
                  //  sky->ComponentChanged(AttributeChange::Local);
                }
                break;       
           }
       case SKYTYPE_DOME:
           {
                break;
           }
       default:
           break;

   }

}

bool Sky::IsSkyEnabled() 
{
    bool exist = false;
    switch (type_ )
    {
        case SKYTYPE_BOX:
            {
                exist = ExistSky<EC_SkyBox >();

                break;
            }
         case SKYTYPE_PLANE:
            {
                exist = ExistSky<EC_SkyPlane >();
                break;
            }
         case SKYTYPE_DOME:
             {
                 break;
             }

    }
    return exist;
   // return skyEnabled_;
}

void Sky::RequestSkyTextures()
{
    boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

    switch(type_)
    {
    case SKYTYPE_BOX:
        for(int i = 0; i < SKYBOX_TEXTURE_COUNT; ++i)
            skyBoxTextureRequests_[i] = renderer->RequestResource(skyBoxTextures_[i],
                OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case SKYTYPE_DOME:
        skyDomeTextureRequest_ = renderer->RequestResource(skyDomeTexture_,
            OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case SKYTYPE_PLANE:
        skyPlaneTextureRequest_ = renderer->RequestResource(skyPlaneTexture_,
            OgreRenderer::OgreTextureResource::GetTypeStatic());
        break;
    case SKYTYPE_NONE:
    default:
        break;
    }
}

void Sky::OnTextureReadyEvent(Resource::Events::ResourceReady *tex)
{
    assert(tex);
    int tags = lstRequestTags_.size();

    for (int i = 0; i < tags; ++i)
    {
        if ( lstRequestTags_[i] == tex->tag_ )
        {
            switch ( type_ )
            {
                 case SKYTYPE_BOX:
                     {
                         EC_SkyBox* sky = GetEnviromentSky<EC_SkyBox >();
                         if ( sky != 0)
                         {
                             QVariantList lstTextures = sky->textureAttr.Get();
                             if ( requestMap_.contains(tex->tag_) )
                             {
                                 int index = requestMap_[tex->tag_];
                                 if ( index >= 0 && index <= 6)
                                 {
                                    lstTextures[requestMap_[tex->tag_]] = QString(tex->id_.c_str());
                                    sky->textureAttr.Set(lstTextures, AttributeChange::Default);
                                  //  sky->ComponentChanged(AttributeChange::Local);
                                 }
                                 else
                                    EnvironmentModule::LogWarning("Tried to change texture which index was higher then 6 or less then 0");

                             }
                                
                         }
                         break;
                     }

                 case SKYTYPE_PLANE:
                     {
                         EC_SkyPlane* sky = GetEnviromentSky<EC_SkyPlane >();
                         if ( sky != 0)
                         {
                             QString texture = sky->textureAttr.Get();
                             QString strDownloaded(tex->id_.c_str());
                             if ( texture != strDownloaded )
                             {
                                 sky->textureAttr.Set(strDownloaded, AttributeChange::Default);
                                // sky->ComponentChanged(AttributeChange::Local);
                             }

                         }
                         
                         break;
                     }
                 case SKYTYPE_DOME:
                     {

                         break;
                     }
                 default:
                     break;


            }

            lstRequestTags_.removeAt(i);
            // Now all done. 
            return;
        }

    }

    /*
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if (!sky)
    {
        EnvironmentModule::LogDebug("Could not get SkyEntityPtr!");
        return;
    }

    EC_OgreSky *sky_component = sky->GetComponent<EC_OgreSky>().get();
    assert(sky_component);

    switch(type_)
    {
    case SKYTYPE_BOX:
        for(int i = 0; i < SKYBOX_TEXTURE_COUNT; ++i)
            if (tex->tag_ == skyBoxTextureRequests_[i])
                sky_component->SetSkyBoxMaterialTexture(i, tex->id_.c_str(), skyBoxImageCount_);
        break;
    case SKYTYPE_DOME:
        if (tex->tag_ == skyDomeTextureRequest_)
            sky_component->SetSkyDomeMaterialTexture(tex->id_.c_str());
        break;
    case SKYTYPE_PLANE:
        if (tex->tag_ == skyPlaneTextureRequest_)
            sky_component->SetSkyPlaneMaterialTexture(tex->id_.c_str());
        break;
    default:
        break;
    }
    EnableSky(sky_component->IsSkyEnabled());
    */
}

void Sky::SetSkyTexture(const RexAssetID &texture_id)
{
    switch(type_)
    {
    case SKYTYPE_DOME:
        skyDomeTexture_ = texture_id;
        break;
    case SKYTYPE_PLANE:
        skyPlaneTexture_ = texture_id;
        break;
    default:
        EnvironmentModule::LogError("SetSkyTexture can be used only for SkyDome and SkyPlane!");
        break;
    }
}
    
void Sky::SetSkyBoxTextures(const RexAssetID textures[SKYBOX_TEXTURE_COUNT])
{
    if (type_ != SKYTYPE_BOX)
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
        ComponentPtr sky_component = entity.GetComponent(EC_OgreSky::TypeNameStatic());
        if (sky_component.get())
            cachedSkyEntity_ = scene->GetEntity(entity.GetId());
    }
}

Scene::EntityWeakPtr Sky::GetSkyEntity()
{
    return cachedSkyEntity_;
}

SkyType Sky::GetSkyType() const
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

void Sky::ChangeSkyType(SkyType type, bool update_sky)
{
    if ( type == type_ && update_sky )
            return;

    DisableSky();
    /*
    switch(type_ )
    {
        switch(type_)
        {
        case OgreRenderer::SKYTYPE_BOX:
            {
                RemoveSky<EC_SkyBox>();
                break;
            }
        case OgreRenderer::SKYTYPE_PLANE:
            {
               RemoveSky<EC_SkyPlane>();
               break;
            }          
        case OgreRenderer::SKYTYPE_DOME:
            //skyDomeTexture_ = sky_component->GetSkyDomeTextureID();
            break;
        }

    }
   */
    switch(type )
    {
      
        case SKYTYPE_BOX:
            {
                CreateSky<EC_SkyBox>();
                EC_SkyBox* box = GetEnviromentSky<EC_SkyBox>();
                //box->ComponentChanged(AttributeChange::Local);
                break;
            }
        case SKYTYPE_PLANE:
            {
               CreateSky<EC_SkyPlane>();
               EC_SkyPlane* plane = GetEnviromentSky<EC_SkyPlane>();
             //  plane->ComponentChanged(AttributeChange::Local);
               break;
            }          
        case SKYTYPE_DOME:
            {
                //skyDomeTexture_ = sky_component->GetSkyDomeTextureID();
                break;
            }

    }
    
    type_ = type;
    //emit SkyTypeChanged();
   

    /*
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        EnableSky(update_sky);
        sky_component->SetSkyType(type, update_sky);
        type_ = type;
        
        switch(type_)
        {
        case SKYTYPE_BOX:
            for(uint i = 0; i < SKYBOX_TEXTURE_COUNT; i++)
                skyBoxTextures_[i] = sky_component->GetSkyBoxTextureID(i);
            break;
        case SKYTYPE_PLANE:
            skyPlaneTexture_ = sky_component->GetSkyDomeTextureID();
            break;
        case SKYTYPE_DOME:
            skyDomeTexture_ = sky_component->GetSkyDomeTextureID();
            break;
        }

        emit SkyTypeChanged();
    }
    */
}

RexTypes::RexAssetID Sky::GetSkyTextureID(SkyType sky_type, int index) const
{
    if(index < 0) index = 0;
    else if(index > SKYBOX_TEXTURE_COUNT - 1) index = SKYBOX_TEXTURE_COUNT - 1;

    if(sky_type == SKYTYPE_BOX)
    {
        return skyBoxTextures_[index];
    }
    else if(sky_type == SKYTYPE_DOME)
    {
        return skyDomeTexture_;
    }
    else if(sky_type == SKYTYPE_PLANE)
    {
        return skyPlaneTexture_;
    }
    return 0;
}

SkyDomeParameters Sky::GetSkyDomeParameters()
{
    SkyDomeParameters sky_dome_param;
    sky_dome_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_dome_param = sky_component->GetSkyDomeParameters();
    }
    return sky_dome_param;
}

SkyPlaneParameters Sky::GetSkyPlaneParameters()
{
    SkyPlaneParameters sky_plane_param;
    sky_plane_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_plane_param = sky_component->GetSkyPlaneParameters();
    }
    return sky_plane_param;
}

SkyBoxParameters Sky::GetSkyBoxParameters()
{
    SkyBoxParameters sky_param;
    sky_param.Reset();
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_param = sky_component->GetBoxSkyParameters();
    }
    return sky_param;
}

void Sky::SetSkyDomeParameters(const SkyDomeParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyDomeParameters(params, update_sky);
        if(update_sky)
        {
            type_ = SKYTYPE_DOME;
            emit SkyTypeChanged();
        }
    }
}

void Sky::SetSkyPlaneParameters(const SkyPlaneParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyPlaneParameters(params, update_sky);
        if(update_sky)
        {
            type_ = SKYTYPE_PLANE;
            emit SkyTypeChanged();
        }
    }
}

void Sky::SetSkyBoxParameters(const SkyBoxParameters &params, bool update_sky)
{
    Scene::EntityPtr sky = GetSkyEntity().lock();
    if(sky)
    {
        EC_OgreSky *sky_component = checked_static_cast<EC_OgreSky*>(sky->GetComponent("EC_OgreSky").get());
        assert(sky_component);
        sky_component->SetSkyBoxParameters(params, update_sky);
        if(update_sky)
        {
            type_ = SKYTYPE_BOX;
            emit SkyTypeChanged();
        }
    }
}

} // namespace RexLogic
