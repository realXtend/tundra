// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Sky_h
# error "Never use <Sky-templates.h> directly; include <Sky.h> instead."
#endif

namespace Environment
{

    template <typename T> T* Sky::GetEnviromentSky()
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

        template <typename T> bool Sky::ExistSky() 
        {
            if ( GetEnviromentSky<T >() != 0 )
                return true;

            return false;
        }


        template <typename T> void Sky::CreateSky()
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
               if ( entity->HasComponent(EC_SkyDome::TypeNameStatic()))
                   entity->RemoveComponent(entity->GetComponent(EC_SkyDome::TypeNameStatic()), AttributeChange::Replicate);
               

           }
           
           QString name = "SkyEnvironment";
           owner_->CreateEnvironmentEntity(name, T::TypeNameStatic());  
           
        }


        template <typename T> void Sky::RemoveSky()
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
}
