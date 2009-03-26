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

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterface_Impl(type_static_)
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
        
        AutoRegisterConsoleCommand(Console::CreateCommand(
            "SetViewportColor", "Set viewport background color. Usage: SetViewportColor(r,g,b)", 
            Console::Bind(this, &OgreRenderingModule::ConsoleSetViewportColor)));
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::Initialize()
    {        
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_));
        renderer_->Initialize();
        
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void OgreRenderingModule::PostInitialize()
    {
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
        //EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_ptr.get());
        //EC_OgreMesh* mesh = static_cast<EC_OgreMesh*>(mesh_ptr.get());
        //placeable->SetPosition(Core::Vector3df(-50,0,-200));
        //mesh->SetMesh("ogrehead.mesh");
        //mesh->SetPlaceable(placeable_ptr);
        //
        //Foundation::EntityPtr entity2 = scene->CreateEntity(2);
        //Foundation::ComponentPtr light_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreLight::NameStatic());
        //entity2->AddEntityComponent(light_ptr);
        //
        //EC_OgreLight* light = static_cast<EC_OgreLight*>(light_ptr.get());
        //light->SetType(EC_OgreLight::LT_Directional);
        //light->SetDirection(Core::Vector3df(-1,-1,-1));
        //
        //Foundation::EntityPtr entity3 = scene->CreateEntity(3);
        //Foundation::ComponentPtr sky_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreSky::NameStatic());
        //entity3->AddEntityComponent(sky_ptr);
        //
        //EC_OgreSky* sky = static_cast<EC_OgreSky*>(sky_ptr.get());
        //sky->SetSkyBox("Sky", 1000);
       
        //Foundation::EntityPtr entity4 = scene->CreateEntity(4);
        //Foundation::ComponentPtr placeable4_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgrePlaceable::NameStatic());
        //Foundation::ComponentPtr custom4_ptr = framework_->GetComponentManager()->CreateComponent(EC_OgreCustomObject::NameStatic());
        //entity4->AddEntityComponent(placeable4_ptr);
        //entity4->AddEntityComponent(custom4_ptr);
        //
        //EC_OgrePlaceable* placeable4 = static_cast<EC_OgrePlaceable*>(placeable4_ptr.get());
        //EC_OgreCustomObject* custom4 = static_cast<EC_OgreCustomObject*>(custom4_ptr.get());
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
    void OgreRenderingModule::Uninitialize()
    {        
        framework_->GetServiceManager()->UnregisterService(renderer_.get());
    
        renderer_.reset();
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void OgreRenderingModule::Update()
    {
        renderer_->Update();
    }

    Console::CommandResult OgreRenderingModule::ConsoleSetViewportColor(const Core::StringVector &params)
    {
        
        if (params.size() != 3)
        {
            return Console::ResultFailure("Usage: SetViewportColor(r,g,b)");
        }

        Ogre::ColourValue color;
        try
        {
            color = Ogre::ColourValue(Core::ParseString<Core::Real>(params[0]), Core::ParseString<Core::Real>(params[1]), Core::ParseString<Core::Real>(params[2]));
        } catch (std::exception)
        {
            return Console::ResultInvalidParameters();
        }

        Ogre::Camera *camera = renderer_->GetCurrentCamera();
        if (camera == NULL)
            return Console::ResultFailure("No camera or viewport");

        camera->getViewport()->setBackgroundColour(color);
        return Console::ResultSuccess();
    }
}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

