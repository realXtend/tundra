/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *  @file   EnvironmentModule.cpp
 *  @brief  Environment module. Environment module is be responsible of visual environment features like terrain, sky & water.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EnvironmentModule.h"
#include "Water.h"
#include "Environment.h"
#include "Sky.h"
#include "EC_WaterPlane.h"
#include "EC_Fog.h"
#include "EC_SkyPlane.h"
#include "EC_SkyBox.h"
#include "EC_SkyDome.h"
#include "EC_EnvironmentLight.h"
#include "TerrainWeightEditor.h"
#include "EC_OgreEnvironment.h"

#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "Renderer.h"

#include "SceneManager.h"
#include "ModuleManager.h"
#include "EventManager.h"
#include "CompositionHandler.h"
#include "EC_Name.h"
#include "EC_Terrain.h"
//#include "WorldBuildingServiceInterface.h"
#include "../TundraLogicModule/TundraEvents.h"

#include "MemoryLeakCheck.h"

namespace Environment
{
    std::string EnvironmentModule::type_name_static_ = "Environment";

    EnvironmentModule::EnvironmentModule() :
        IModule(type_name_static_),
        terrainWeightEditor_(0)
    {
    }

    EnvironmentModule::~EnvironmentModule()
    {
    }

    void EnvironmentModule::Load()
    {
        DECLARE_MODULE_EC(EC_Terrain);
        DECLARE_MODULE_EC(EC_WaterPlane);
        DECLARE_MODULE_EC(EC_Fog);
        DECLARE_MODULE_EC(EC_SkyPlane);
        DECLARE_MODULE_EC(EC_SkyBox);
        DECLARE_MODULE_EC(EC_SkyDome);
        DECLARE_MODULE_EC(EC_EnvironmentLight);

        /// Create an asset type factory for Terrain assets. The terrain assets are handled as binary blobs - the EC_Terrain parses it when showing the asset.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Terrain")));
    }

    void EnvironmentModule::Initialize()
    {
    }

    void EnvironmentModule::PostInitialize()
    {
        event_manager_ = framework_->GetEventManager();
        
        event_manager_->RegisterEventSubscriber(this, 99);
        tundra_event_category_ = event_manager_->QueryEventCategory("Tundra");
    }

    void EnvironmentModule::Uninitialize()
    {
        SAFE_DELETE(terrainWeightEditor_);
        water_.reset();
        environment_.reset();
        sky_.reset();
        event_manager_.reset();
    }

    void EnvironmentModule::Update(f64 frametime)
    {
        RESETPROFILER;
        PROFILE(EnvironmentModule_Update);

        if (GetFramework()->Scene()->GetDefaultScene())
        {
            if (environment_.get() != 0)
                environment_->Update(frametime);
            if (water_.get() !=0 )
                water_->Update();
            //if  (sky_.get() != 0)
            //    sky_->Update();
        }
    }

#ifdef CAELUM
    Caelum::CaelumSystem* EnvironmentModule::GetCaelum()
    {   
         if (environment_.get() != 0)
         {
            EC_OgreEnvironment* ev = environment_->GetEnvironmentComponent();
            if (ev != 0)
                return ev->GetCaelum();
         }

         return 0;
    }
#endif

    void EnvironmentModule::ShowTerrainWeightEditor()
    {
        if (framework_->IsHeadless())
            return;

        if (terrainWeightEditor_)
        {
            terrainWeightEditor_->show();
            return;
        }

        terrainWeightEditor_ = new TerrainWeightEditor(framework_);
        terrainWeightEditor_->setWindowFlags(Qt::Tool);
        terrainWeightEditor_->show();
    }

    bool EnvironmentModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
            if (category_id == tundra_event_category_)
        {
            if (event_id == TundraLogic::Events::EVENT_TUNDRA_CONNECTED)
            {
                Scene::ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
                if (scene)
                {
                    CreateEnvironment();
                    //CreateTerrain();
                    //CreateWater();
                    //CreateEnvironment();
                    //CreateSky();
                }
                // Hook to the SceneCleared event to recreate the necessary environment (Caelum) when scene is cleared
                /// \todo Temp solution, will be removed
                connect(scene.get(), SIGNAL(SceneCleared(Scene::SceneManager*)), this, SLOT(OnSceneCleared(Scene::SceneManager*)));
            }

            if (event_id == TundraLogic::Events::EVENT_TUNDRA_DISCONNECTED)
            {
                ReleaseEnvironment();
            }
        }

        return false;
    }

    ///\todo Delete this. -jj. 
    Scene::EntityPtr EnvironmentModule::CreateEnvironmentEntity(const QString& entity_name, const QString& component_name) 
    {
        
        Scene::ScenePtr active_scene = GetFramework()->Scene()->GetDefaultScene();
        // Search first that does there exist environment entity
        Scene::EntityPtr entity = active_scene->GetEntityByName(entity_name);
        if (entity != 0)
        {
            // Does it have component? If not create. 
            if (!entity->HasComponent(component_name) )
                entity->AddComponent(framework_->GetComponentManager()->CreateComponent(component_name), AttributeChange::Replicate);
        
        
            return entity;
        }
       

        entity = active_scene->GetEntityByName("LocalEnvironment");

        if (entity != 0)
        {
             // Does it have component? If not create. 
            if (!entity->HasComponent(component_name) )
                entity->AddComponent(framework_->GetComponentManager()->CreateComponent(component_name), AttributeChange::LocalOnly);

        }
        else
        {
            int id = active_scene->GetNextFreeIdLocal();
            entity = active_scene->CreateEntity(id);
            entity->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_Name::TypeNameStatic()));
            EC_Name* nameComp = entity->GetComponent<EC_Name >().get();
            nameComp->name.Set("LocalEnvironment", AttributeChange::LocalOnly);
            
            // Create param component.
            entity->AddComponent(framework_->GetComponentManager()->CreateComponent(component_name), AttributeChange::LocalOnly);
        }

        return entity;
    }

    ///\todo Delete this. -jj.
    void EnvironmentModule::RemoveLocalEnvironment()
    {
        Scene::ScenePtr active_scene = GetFramework()->Scene()->GetDefaultScene();
        Scene::Entity* entity = active_scene->GetEntityByName("LocalEnvironment").get();
    
        if (entity == 0)
            return;
        else
        {   
            if (entity->HasComponent(EC_WaterPlane::TypeNameStatic()) && active_scene->GetEntityByName("WaterEnvironment").get() != 0 )
                entity->RemoveComponent(entity->GetComponent(EC_WaterPlane::TypeNameStatic()));  
            if  ( entity->HasComponent(EC_Fog::TypeNameStatic()) && active_scene->GetEntityByName("FogEnvironment").get() != 0)
                 entity->RemoveComponent(entity->GetComponent(EC_Fog::TypeNameStatic()));
            if (entity->HasComponent(EC_SkyPlane::TypeNameStatic()) && active_scene->GetEntityByName("SkyEnvironment").get() != 0)
                entity->RemoveComponent(entity->GetComponent(EC_SkyPlane::TypeNameStatic()));
            if (entity->HasComponent(EC_SkyBox::TypeNameStatic()) && active_scene->GetEntityByName("SkyEnvironment").get() != 0)
                entity->RemoveComponent(entity->GetComponent(EC_SkyBox::TypeNameStatic()));
             if (entity->HasComponent(EC_SkyDome::TypeNameStatic()) && active_scene->GetEntityByName("SkyEnvironment").get() != 0)
                entity->RemoveComponent(entity->GetComponent(EC_SkyDome::TypeNameStatic()));
            if (entity->HasComponent(EC_EnvironmentLight::TypeNameStatic()) && active_scene->GetEntityByName("LightEnvironment").get() != 0)
                entity->RemoveComponent(entity->GetComponent(EC_EnvironmentLight::TypeNameStatic()));
        }

        if (!entity->HasComponent(EC_WaterPlane::TypeNameStatic()) &&
            !entity->HasComponent(EC_Fog::TypeNameStatic())  && 
            !entity->HasComponent(EC_SkyPlane::TypeNameStatic()) && 
            !entity->HasComponent(EC_SkyBox::TypeNameStatic()) && 
            !entity->HasComponent(EC_EnvironmentLight::TypeNameStatic()) &&
            !entity->HasComponent(EC_SkyDome::TypeNameStatic()))
        {
            active_scene->RemoveEntity(entity->GetId());
        }
    }

    void EnvironmentModule::CreateEnvironment()
    {
        environment_ = EnvironmentPtr(new Environment(this));
        environment_->CreateEnvironment();
    }

    void EnvironmentModule::ReleaseEnvironment()
    {
        environment_.reset();
    }

    void EnvironmentModule::OnSceneCleared(Scene::SceneManager* scene)
    {
        if (scene)
            CreateEnvironment();
    }
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new Environment::EnvironmentModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
