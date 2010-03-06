// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "TestModule.h"
#include "EventManager.h"
#include "EC_Dummy.h"
//#include <Poco/ClassLibrary.h>
#include "TestServiceInterface.h"
#include "TestService.h"
#include "SceneManager.h"
#include "ConfigurationManager.h"

namespace Test
{
    TestModule::TestModule() : ModuleInterfaceImpl(type_static_)
    {
        test_service_ = TestServicePtr(new TestService);
    }

    TestModule::~TestModule()
    {
    }

    // virtual
    void TestModule::Load()
    {
        DECLARE_MODULE_EC(EC_Dummy);

        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void TestModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void TestModule::Initialize()
    {
        framework_->GetServiceManager()->RegisterService(TestService::type_, test_service_);
        assert (framework_->GetServiceManager()->IsRegistered(TestService::type_) &&
            "Failed to register test service");

        framework_->GetEventManager()->RegisterEventCategory("Test");

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void TestModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(test_service_);
        assert (framework_->GetServiceManager()->IsRegistered(TestService::type_) == false &&
            "Failed to unregister test service");

        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void TestModule::Update(f64 frametime)
    {
        LogInfo("");

        assert (VersionMajor() == GetFramework()->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "version_major"));
        assert (VersionMinor() == GetFramework()->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "version_minor"));

        // create new entity
        LogInfo("Constructing entity with component: " + Test::EC_Dummy::NameStatic() + ".");

        Foundation::Framework *sceneManager = framework_;

        assert(sceneManager->HasScene("test_scene") == false && "Scene test_scene scene already exists!");
        Scene::ScenePtr scene = sceneManager->CreateScene("test_scene");
        assert(scene.get() && "Failed to create scene" );
        assert(sceneManager->HasScene("test_scene") && "Failed to create scene");

        Scene::EntityPtr entity = scene->CreateEntity(0);
        assert (entity.get() != 0 && "Failed to create entity.");
        assert (scene->HasEntity(entity->GetId()) && "Failed to add entity to scene properly!");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::NameStatic());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->AddComponent(component);
        component = entity->GetComponent(component->Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::ComponentPtr component_second = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::NameStatic());
        entity->AddComponent(component_second);
        component = entity->GetComponent(component_second->Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");
        entity->RemoveComponent(component_second);
        component_second.reset();

/*
        int num_test_components = 0;
        for (Foundation::ComponentManager::const_iterator it = 
             framework_->GetComponentManager()->Begin(Test::EC_Dummy::NameStatic()) ;
             it != framework_->GetComponentManager()->End(Test::EC_Dummy::NameStatic()) ;
             ++it)
        {
            assert (it->lock()->Name() == Test::EC_Dummy::NameStatic() && "Component iterator returned wrong component type.");
            num_test_components++;
        }
        assert (num_test_components == 1 && "Component iterator failed.");
*/

        Scene::ScenePtr cloned_scene = scene->Clone("Test clone scene");
        assert (sceneManager->HasScene("Test clone scene"));
        assert (cloned_scene->HasEntity(entity->GetId()) && "Failed to clone a scene");

        //Scene::EntityPtr cloned_entity = entity->Clone(cloned_scene);
        Scene::EntityPtr cloned_entity = cloned_scene->GetEntity(entity->GetId());
        component = cloned_entity->GetComponent(component->Name());
        assert (component.get() != 0 && "Failed to clone an entity.");

        Scene::EntityPtr cloned_entity2 = cloned_scene->GetEntity(cloned_entity->GetId());
        Scene::EntityPtr entity2 = cloned_scene->CreateEntity(0);
        assert (*cloned_entity2.get() == *cloned_entity.get() && "EntityInterface operator== failed");
        assert (*entity.get() != *entity2.get() && "EntityInterface operator!= failed");
        assert (*cloned_entity.get() < *entity2.get() && "EntityInterface operator< failed");


        Scene::ScenePtr scene2 = sceneManager->GetScene("test_scene");
        assert(*scene.get() == *scene2.get() && "SceneManager operator== failed");
        assert(*scene.get() != *cloned_scene.get() && "SceneManager operator!= failed");
        assert(*cloned_scene.get() < *scene.get() && "SceneManager operator< failed");


        boost::shared_ptr<TestServiceInterface> test_service = framework_->GetServiceManager()->GetService<TestServiceInterface>(TestService::type_).lock();
        assert (test_service);
        assert (test_service->DoTest());


        TestEvent test_event;
        test_event.test_value_ = 12345;
        framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("Test"), 0, NULL);
        framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("Test"), 1, &test_event);

        framework_->Exit();
        assert (framework_->IsExiting());

        LogInfo("");
    }
}

