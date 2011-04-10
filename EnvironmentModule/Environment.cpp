/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Environment.cpp
 *  @brief  Manages environment-related logic, e.g. world time and lighting.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Environment.h"
#include "EnvironmentModule.h"
#include "EC_WaterPlane.h"
#include "EC_Fog.h"

#include "EC_OgreEnvironment.h"
#include "SceneManager.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "Renderer.h"

#ifdef CAELUM
#include <Caelum.h>
#endif

#include "MemoryLeakCheck.h"

namespace Environment
{

/// Utility tool for clamping fog distance
void ClampFog(float& start, float& end, float farclip)
{
    if (farclip < 10.0) 
        farclip = 10.0;
    if (end > farclip - 10.0)
        end = farclip - 10.0;
    if (start > farclip/3.0)
        start = farclip/3.0;
}

Environment::Environment(EnvironmentModule *owner) :
    owner_(owner)
{

#ifdef CAELUM
    caelumComponents_ = Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE;
    caelumComponents_ = caelumComponents_ |
        Caelum::CaelumSystem::CAELUM_COMPONENT_SKY_DOME |
        Caelum::CaelumSystem::CAELUM_COMPONENT_MOON |
        Caelum::CaelumSystem::CAELUM_COMPONENT_SUN |
        Caelum::CaelumSystem::CAELUM_COMPONENT_POINT_STARFIELD |
        Caelum::CaelumSystem::CAELUM_COMPONENT_SCREEN_SPACE_FOG |
        Caelum::CaelumSystem::CAELUM_COMPONENT_GROUND_FOG;
#endif
}

Environment::~Environment()
{
    if (activeEnvEntity_.lock().get() != 0)
    {
        EntityPtr entity = activeEnvEntity_.lock();
        if (entity != 0)
        {
            // Remove entity from scene.
            Scene::SceneManager* manager = entity->GetScene();
            if (manager != 0)
                manager->RemoveEntity(entity->GetId());
        }

        entity.reset();
    }

    activeEnvEntity_.reset();
    owner_ = 0;
}

EC_OgreEnvironment* Environment::GetEnvironmentComponent()
{
    if (activeEnvEntity_.expired())
        return 0;

    return activeEnvEntity_.lock()->GetComponent<EC_OgreEnvironment>().get();  
}

void Environment::CreateEnvironment()
{
    Scene::ScenePtr active_scene = owner_->GetFramework()->Scene()->GetDefaultScene();
    EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeIdLocal());

    // The environment entity exists due to legacy from Taiga. It is bad design to create the entity here.
    entity->SetTemporary(true);
    active_scene->EmitEntityCreated(entity);
    activeEnvEntity_ = entity;

    // Creates Caelum component!!
    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreEnvironment"));
    
    // Creates default fog component
    
    // Does there exist allready Enviroment-entity?
    //Scene::ScenePtr active_scene = owner_->GetFramework()->Scene()->GetDefaultScene();
    
    Scene::Entity* enviroment = active_scene->GetEntityByName("FogEnvironment").get();
    
    if (enviroment != 0 )
    {
        owner_->RemoveLocalEnvironment();
        
        // Has it allready EC_Fog ?
        if (!enviroment->HasComponent(EC_Fog::TypeNameStatic()))
            enviroment->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_Fog::TypeNameStatic())); 
    }
    else
    {
        enviroment =  active_scene->GetEntityByName("LocalEnvironment").get();
        if (enviroment != 0 && !enviroment->HasComponent(EC_Fog::TypeNameStatic()))
            enviroment->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_Fog::TypeNameStatic())); 
    }
}

EC_EnvironmentLight* Environment::GetEnvironmentLight()
{
    Scene::ScenePtr active_scene = owner_->GetFramework()->Scene()->GetDefaultScene();
    Scene::Entity* entity = active_scene->GetEntityByName("LightEnvironment").get();
    
    if (entity != 0 )
    {
        owner_->RemoveLocalEnvironment();
        if (!entity->HasComponent(EC_EnvironmentLight::TypeNameStatic()) )
            entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_EnvironmentLight::TypeNameStatic()));
    }
    else
    {
        entity =  active_scene->GetEntityByName("LocalEnvironment").get();
        
        if (entity == 0 || !entity->HasComponent(EC_EnvironmentLight::TypeNameStatic()))
        {
            ///todo Search first entity which has light environment component! this might be slow, work around somehow?
            EntityList lst = active_scene->GetEntitiesWithComponent(EC_EnvironmentLight::TypeNameStatic());
            if (lst.size() == 0 )
                return 0;
            
            entity = lst.front().get();
        }
        
       
    }

    return entity->GetComponent<EC_EnvironmentLight >().get();
}

void Environment::Update(f64 frametime)
{
    PROFILE(Environment_Update);
}

bool Environment::IsCaelum()
{
    EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false;

    return env->IsCaelumUsed();
}

}
