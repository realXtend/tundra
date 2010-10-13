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
        
Sky::Sky(EnvironmentModule *owner) : owner_(owner), skyEnabled_(false), type_(SKYTYPE_BOX) 
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
       
    }

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
            if ( !ExistSky<EC_SkyDome>())
                CreateSky<EC_SkyDome>();
            
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
  

    emit SkyTypeChanged();
}

void Sky::CreateDefaultSky(const bool &show)
{
    type_ = SKYTYPE_BOX;
    // Exist sky? 
    if ( !ExistSky<EC_SkyBox>() )
        CreateSky<EC_SkyBox>();

    EnableSky(true);
 
}

void Sky::DisableSky()
{
    RemoveSky<EC_SkyBox>();
    RemoveSky<EC_SkyPlane>();
    RemoveSky<EC_SkyDome >();
    
    
   
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
               
                }
                break;       
           }
       case SKYTYPE_DOME:
           {
                EC_SkyDome* sky = GetEnviromentSky<EC_SkyDome >();
                if (sky == 0)
                {
                    CreateSky<EC_SkyDome>();
                    EC_SkyDome* sky = GetEnviromentSky<EC_SkyDome >();
                
                }
               
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
                 exist = ExistSky<EC_SkyDome >();
                 break;
             }

    }
    return exist;
   
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
                               
                             }

                         }
                         
                         break;
                     }
                 case SKYTYPE_DOME:
                     {
                         EC_SkyDome* sky = GetEnviromentSky<EC_SkyDome >();
                         if ( sky != 0)
                         {
                             QString texture = sky->textureAttr.Get();
                             QString strDownloaded(tex->id_.c_str());
                             if ( texture != strDownloaded )
                             {
                                 sky->textureAttr.Set(strDownloaded, AttributeChange::Default);
                                
                             }

                         }

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
    
    switch(type )
    {
      
        case SKYTYPE_BOX:
            {
                CreateSky<EC_SkyBox>();
                EC_SkyBox* box = GetEnviromentSky<EC_SkyBox>();
               
                break;
            }
        case SKYTYPE_PLANE:
            {
               CreateSky<EC_SkyPlane>();
               EC_SkyPlane* plane = GetEnviromentSky<EC_SkyPlane>();
        
               break;
            }          
        case SKYTYPE_DOME:
            {
                CreateSky<EC_SkyDome>();
                EC_SkyDome* plane = GetEnviromentSky<EC_SkyDome >();
        
                break;
            }

    }
    
    type_ = type;
  
}

} // namespace RexLogic
