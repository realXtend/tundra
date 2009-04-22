// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreLight.h"
#include "EC_OgreSky.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreConsoleOverlay.h"

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterfaceImpl(type_static_),
        assetcategory_id_(0),
        resourcecategory_id_(0)
    {
    }

    OgreRenderingModule::~OgreRenderingModule()
    {
    }

    // virtual
    void OgreRenderingModule::Load()
    {
        using namespace OgreRenderer;

        LogInfo("Module " + Name() + " loaded.");

        DECLARE_MODULE_EC(EC_OgrePlaceable);
        DECLARE_MODULE_EC(EC_OgreMesh);
        DECLARE_MODULE_EC(EC_OgreLight);
        DECLARE_MODULE_EC(EC_OgreSky);
        DECLARE_MODULE_EC(EC_OgreCustomObject);
        DECLARE_MODULE_EC(EC_OgreConsoleOverlay);
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::PreInitialize()
    {
        // create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_));
    }

    // virtual
    void OgreRenderingModule::Initialize()
    {      
        assert (renderer_);
        renderer_->Initialize();
        
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void OgreRenderingModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        assetcategory_id_ = event_manager->QueryEventCategory("Asset");
        
        if (assetcategory_id_ == 0 )
            LogWarning("Unable to find event category for Asset events!");

        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
        if (!resourcecategory_id_)
        {
            resourcecategory_id_ = event_manager->RegisterEventCategory("Resource");
            event_manager->RegisterEvent(resourcecategory_id_, Resource::Event::RESOURCE_READY, "ResourceReady");
        }

        renderer_->PostInitialize();

        // Hackish way to register renderer debug console commands
        if (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_ConsoleCommand))
        {
            Console::CommandService *console = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand);
            console->RegisterCommand(Console::CreateCommand(
                "RequestTexture", "Download, decode & create Ogre texture. Usage: RequestTexture(uuid)", 
                Console::Bind(this, &OgreRenderingModule::ConsoleRequestTexture)));
            console->RegisterCommand(Console::CreateCommand(
                "RequestMesh", "Download & create Ogre mesh. Usage: RequestMesh(uuid)", 
                Console::Bind(this, &OgreRenderingModule::ConsoleRequestMesh)));
        }

        //Foundation::SceneManagerServiceInterface *scene_manager = 
        //    framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        //assert(scene_manager != NULL && "Failed to get SceneManager service");
        //
        //if (scene_manager->HasScene("World") == false)
        //    scene_manager->CreateScene("World");
        //Foundation::ScenePtr scene = scene_manager->GetScene("World");
        //
        //Foundation::EntityPtr entity = scene->CreateEntity(1);
        //Foundation::ComponentPtr placeable_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgrePlaceable::NameStatic());
        //Foundation::ComponentPtr mesh_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreMesh::NameStatic());
        //entity->AddEntityComponent(placeable_ptr);
        //entity->AddEntityComponent(mesh_ptr);
        //
        //EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_ptr.get());
        //EC_OgreMesh* mesh = checked_static_cast<EC_OgreMesh*>(mesh_ptr.get());
        //placeable->SetPosition(Core::Vector3df(-50,0,-200));
        //mesh->SetMesh("ogrehead.mesh");
        //mesh->SetPlaceable(placeable_ptr);
        //
        //Foundation::EntityPtr entity2 = scene->CreateEntity(2);
        //Foundation::ComponentPtr light_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreLight::NameStatic());
        //entity2->AddEntityComponent(light_ptr);
        //
        //EC_OgreLight* light = checked_static_cast<EC_OgreLight*>(light_ptr.get());
        //light->SetType(EC_OgreLight::LT_Directional);
        //light->SetDirection(Core::Vector3df(-1,-1,-1));
        //
        //Foundation::EntityPtr entity3 = scene->CreateEntity(3);
        //Foundation::ComponentPtr sky_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreSky::NameStatic());
        //entity3->AddEntityComponent(sky_ptr);
        //
        //EC_OgreSky* sky = checked_static_cast<EC_OgreSky*>(sky_ptr.get());
        //sky->SetSkyBox("Sky", 1000);
       
        //Foundation::EntityPtr entity4 = scene->CreateEntity(4);
        //Foundation::ComponentPtr placeable4_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgrePlaceable::NameStatic());
        //Foundation::ComponentPtr custom4_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreCustomObject::NameStatic());
        //entity4->AddEntityComponent(placeable4_ptr);
        //entity4->AddEntityComponent(custom4_ptr);
        //
        //EC_OgrePlaceable* placeable4 = checked_static_cast<EC_OgrePlaceable*>(placeable4_ptr.get());
        //EC_OgreCustomObject* custom4 = checked_static_cast<EC_OgreCustomObject*>(custom4_ptr.get());
        //placeable4->SetPosition(Core::Vector3df(50,0,-200));
        //custom4->SetPlaceable(placeable4_ptr);
        //
        //placeable4->SetParent(placeable_ptr);
        //
        //Ogre::ManualObject* manual = custom4->GetObject();
        //manual->begin("BaseWhiteNoLighting");
        //manual->position(Ogre::Vector3(-50,-50,0));
        //manual->position(Ogre::Vector3(50,-50,0));
        //manual->position(Ogre::Vector3(50,50,0));
        //manual->position(Ogre::Vector3(-50,50,0));
        //manual->triangle(0,1,2);
        //manual->triangle(0,2,3);
        //manual->end();
        //
        //renderer_->GetSceneManager()->setAmbientLight(Ogre::ColourValue(0.1,0.1,0.1));
    }

    // virtual
    bool OgreRenderingModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        if (!renderer_)
            return false;

        if (category_id == assetcategory_id_)
        {
            return renderer_->HandleAssetEvent(event_id, data);
        }

        if (category_id == resourcecategory_id_)
        {
            return renderer_->HandleResourceEvent(event_id, data);
        }

        return false;
    }

    // virtual 
    void OgreRenderingModule::Uninitialize()
    {        
        framework_->GetServiceManager()->UnregisterService(renderer_.get());
    
        renderer_.reset();
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void OgreRenderingModule::Update(Core::f64 frametime)
    {
        renderer_->Update(frametime);
    }

    Console::CommandResult OgreRenderingModule::ConsoleRequestTexture(const Core::StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("Usage: RequestTexture(uuid)");

        if (renderer_)
            renderer_->RequestTexture(params[0]);

        return Console::ResultSuccess();
    }

    Console::CommandResult OgreRenderingModule::ConsoleRequestMesh(const Core::StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("Usage: RequestMesh(uuid)");

        if (renderer_)
            renderer_->RequestMesh(params[0]);

        return Console::ResultSuccess();
    }
}


using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

