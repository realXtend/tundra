// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Sky_h
#define incl_RexLogicModule_Sky_h

#include "EC_OgreSky.h"
#include "EnvironmentModuleApi.h"
#include "EnvironmentModule.h"
#include "EC_SkyPlane.h"
#include "EC_SkyBox.h"
#include "SceneManager.h"
#include "Entity.h"

#include <QObject>

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

namespace ProtocolUtilities
{
    class NetworkEventInboundData;
}

namespace Environment
{
    class EnvironmentModule;

    /// The OGRE SkyBox has 6 textures: front, back, left, right, top and bottom.
    const int SKYBOX_TEXTURE_COUNT = 6;

    /// Sky component
    /// \ingroup EnvironmentModuleClient.
    class ENVIRONMENT_MODULE_API Sky : public QObject
    {
        Q_OBJECT

    public:
        Sky(EnvironmentModule *owner);
        virtual ~Sky();

        void Update();

        /// Handler for the "RexSky" generic message.
        /// @param data Event data pointer.
        bool HandleRexGM_RexSky(ProtocolUtilities::NetworkEventInboundData* data);

        /** Update sky info.
         * @param type Type of the sky: box, dome, plane or none. 
         * @param images List of image uuid's for the sky.
         * @param curvature If sky type is dome, curvature of the dome.
         * @param tiling If sky type is dome, tiling of the dome.
         */
        void UpdateSky(const SkyType &type, std::vector<std::string> images,
            const float &curvature, const float &tiling);

        /// Creates the default sky.
        /// @param show Visibility of the sky.
        void CreateDefaultSky(const bool &show = true);

        /// Request the texture assets used for the sky.
        void RequestSkyTextures();

        /// Called whenever a texture is loaded so it can be attached to the sky.
        /// @param Resource pointer to the texture.
        void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);

        /// Set the sky texture for Skydome or Skyplane.
        /// @param texture_id
        void SetSkyTexture(const RexTypes::RexAssetID &texture_id);

        /// Sets the SkyBox textures.
        /// @param textures array of texture UUID's.
        void SetSkyBoxTextures(const RexTypes::RexAssetID textures[SKYBOX_TEXTURE_COUNT]);

        /// Looks through all the entities in RexLogic's currently active scene to find the Sky
        /// entity. Caches it internally. Use GetSkyEntity to obtain it afterwards.    
        void FindCurrentlyActiveSky();

        /// @return The scene entity that represents the sky in the currently active world.
        Scene::EntityWeakPtr GetSkyEntity();

        /// @return The sky type that is in use.
        SkyType GetSkyType() const;

        /// Disable currently active sky.
        void DisableSky();

        /// Return that if sky is enabled.
        bool IsSkyEnabled();

        /// Enable/Disable sky.
        void EnableSky(bool enabled);

        /// Request sky type chaged. If requested sky type is same as currently used sky do nothing.
        void ChangeSkyType(SkyType type, bool update_sky = true);

        /// GetSkyTexture asset id.
        /// @param What sky type we are requesting texture from.
        /// @param Used only when sky type is set to SKYBOX. Index should be between 0 - 5.
        RexTypes::RexAssetID GetSkyTextureID(SkyType sky_type, int index = 0) const;

        /// Return all sky dome parameters from EC_Ogresky entity.
        SkyDomeParameters GetSkyDomeParameters();

        /// Return all sky plane parameters from EC_Ogresky entity.
        SkyPlaneParameters GetSkyPlaneParameters();

        /// Return all generic sky parameters from EC_Ogresky entity.
        SkyBoxParameters GetSkyBoxParameters();

        /// Update sky dome parameters.
        /// @param params contain all information that is need to create a new ogre sky geometry.
        /// @param update_sky do we need to recreate a new sky.
        void SetSkyDomeParameters(const SkyDomeParameters &params, bool update_sky = true);

        /// Update sky plane parameters.
        /// @param params contain all information that is need to create a new ogre sky geometry.
        /// @param update_sky do we need to recreate a new sky.
        void SetSkyPlaneParameters(const SkyPlaneParameters &params, bool update_sky = true);

        /// Update sky box parameters.
        /// @param params contain all information that is need to create a new ogre sky geometry.
        /// @param update_sky do we need to recreate a new sky.
        void SetSkyBoxParameters(const SkyBoxParameters &params, bool update_sky = true);

        template <typename T> T* GetEnviromentSky()
        {
            Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
            Scene::Entity* entity = active_scene->GetEntityByName("SkyEnvironment").get();
    
            if (entity != 0 )
                owner_->RemoveLocalEnvironment();
            else
            {
                entity =  active_scene->GetEntityByName("LocalEnvironment").get();
                if ( entity == 0)
                    return 0;
        
            }
            
            T* sky = entity->GetComponent<T >().get();
    
            return sky;

        }

       

    signals:
        /// Signal is emited when sky is enabled/disabled.
        void SkyEnabled(bool enabled);

        /// Signal is emited when sky type has changed.
        void SkyTypeChanged();

    private:
        Sky(const Sky &);
        void operator =(const Sky &);

        /// RexLogicModule pointer.
        EnvironmentModule *owner_;

        
       
        template <typename T> bool ExistSky() 
        {
            if ( GetEnviromentSky<T >() != 0 )
                return true;

            return false;
        }

        template <typename T> void CreateSky()
        {

           Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
           Scene::Entity* entity = active_scene->GetEntityByName("SkyEnvironment").get();
           
           // First check that does there exist diffrent skies allready?
           if ( entity != 0  )
           {
               // Remove all other skies!
               if ( entity->HasComponent(EC_SkyPlane::TypeNameStatic()))
                   entity->RemoveComponent(entity->GetComponent(EC_SkyPlane::TypeNameStatic()), AttributeChange::Replicate);
               if ( entity->HasComponent(EC_SkyBox::TypeNameStatic()))
                   entity->RemoveComponent(entity->GetComponent(EC_SkyBox::TypeNameStatic()), AttributeChange::Replicate);
               

           }
           
           QString name = "SkyEnvironment";
           owner_->CreateEnvironmentEntity(name, T::TypeNameStatic());  
           
        }
        
        template <typename T> void RemoveSky()
        {
            T* sky = GetEnviromentSky<T>();
            
            if ( sky != 0)
            {
                sky->DisableSky();
            }
            else 
                return;

           Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
           Scene::Entity* entity = active_scene->GetEntityByName("SkyEnvironment").get();
           if (entity == 0)
           {
               entity = active_scene->GetEntityByName("LocalEnvironment").get();
               if ( entity == 0)
                   return;

               // Local environent! !
               if ( entity->HasComponent(T::TypeNameStatic()) )
               {
                    entity->RemoveComponent(entity->GetComponent(T::TypeNameStatic()));
               }
               return;
           }
         
           if ( entity->HasComponent(T::TypeNameStatic()) )
           {
                entity->RemoveComponent(entity->GetComponent(T::TypeNameStatic()), AttributeChange::Replicate);
           }
        }


        /// The type of the sky (none, box, dome or plane).
        SkyType type_;

        /// Whether sky is enabled by this component.
        bool skyEnabled_;

        /// Texture resource request tags for skybox.
        request_tag_t skyBoxTextureRequests_[SKYBOX_TEXTURE_COUNT];

        /// Texture resource request tag for skydome.
        request_tag_t skyDomeTextureRequest_;

        /// Texture resource request tag for skyplane.
        request_tag_t skyPlaneTextureRequest_;

        /// UUID's of the texture assets the skybox uses for rendering. Should be stored per-scene.
        RexTypes::RexAssetID skyBoxTextures_[SKYBOX_TEXTURE_COUNT];

        /// UUID of the texture asset the skydome uses for rendering. Should be stored per-scene.
        RexTypes::RexAssetID skyDomeTexture_;

        /// UUID of the texture asset the skyplane uses for rendering. Should be stored per-scene.
        RexTypes::RexAssetID skyPlaneTexture_;

        /// Keeps count of the skybox images.
        size_t skyBoxImageCount_;

        /// Keeps count of the currently to-be-added skybox images.
        size_t currentSkyBoxImageCount_;

        /// The scene entity that represents the sky in the currently active world.
        Scene::EntityWeakPtr cachedSkyEntity_;

        QList<request_tag_t > lstRequestTags_;
        QMap<int, int >  requestMap_;
    };
}

#endif
