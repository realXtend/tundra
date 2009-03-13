// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "TestModule.h"
#include "EC_Dummy.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "EntityInterface.h"


namespace Test
{
    TestModule::TestModule() : ModuleInterface_Impl(type_static_)
    {
    }

    TestModule::~TestModule()
    {
    }

    // virtual
    void TestModule::Load()
    {
        using namespace Test;
        DECLARE_MODULE_EC(EC_Dummy);

        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void TestModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void TestModule::Initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        framework_ = framework;
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Test, &test_service_);
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void TestModule::Uninitialize(Foundation::Framework *framework)
    {
        framework_->GetServiceManager()->UnregisterService(&test_service_);

        assert(framework_ != NULL);
        framework_ = NULL;
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void TestModule::Update()
    {
        LogInfo("");

        // create new entity
        LogInfo("Constructing entity with component: " + Test::EC_Dummy::Name() + ".");

        Foundation::SceneManagerServiceInterface *sceneManager = 
            framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        assert(sceneManager != NULL && "Failed to get SceneManager service");

        assert(sceneManager->HasScene("test_scene") == false && "Scene test_scene scene alread exists!");
        Foundation::ScenePtr scene = sceneManager->CreateScene("test_scene");
        assert(scene.get() && "Failed to create scene" );
        assert(sceneManager->HasScene("test_scene") && "Failed to create scene");

        Foundation::EntityPtr entity = scene->CreateEntity();
        assert (entity.get() != 0 && "Failed to create entity.");
        assert (scene->HasEntity(entity->GetId()) && "Failed to add entity to scene properly!");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::Name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->AddEntityComponent(component);
        component = entity->GetComponent(component->_Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::ScenePtr cloned_scene = scene->Clone("Test clone scene");
        assert (sceneManager->HasScene("Test clone scene"));
        assert (cloned_scene->HasEntity(entity->GetId()));



        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());

        framework_->Exit();
        assert (framework_->IsExiting());

        LogInfo("");
    }
}

