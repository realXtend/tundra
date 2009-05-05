// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "TestModule.h"
#include "EC_Dummy.h"
#include <Poco/ClassLibrary.h>
#include "EntityInterface.h"



namespace Test
{
    TestModule::TestModule() : ModuleInterfaceImpl(type_static_)
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
    void TestModule::Initialize()
    {
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Test, &test_service_);
        assert (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_Test) &&
            "Failed to register test service");

        framework_->GetEventManager()->RegisterEventCategory("Test");

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void TestModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(&test_service_);
        assert (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_Test) == false &&
            "Failed to unregister test service");

        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void TestModule::Update(Core::f64 frametime)
    {
        LogInfo("");

        assert (VersionMajor() == GetFramework()->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "version_major"));
        assert (VersionMinor() == GetFramework()->GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "version_minor"));

        // create new entity
        LogInfo("Constructing entity with component: " + Test::EC_Dummy::NameStatic() + ".");

        Foundation::SceneManagerServiceInterface *sceneManager = 
            framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
        assert(sceneManager != NULL && "Failed to get SceneManager service");

        assert(sceneManager->HasScene("test_scene") == false && "Scene test_scene scene already exists!");
        Foundation::ScenePtr scene = sceneManager->CreateScene("test_scene");
        assert(scene.get() && "Failed to create scene" );
        assert(sceneManager->HasScene("test_scene") && "Failed to create scene");

        Foundation::EntityPtr entity = scene->CreateEntity(0);
        assert (entity.get() != 0 && "Failed to create entity.");
        assert (scene->HasEntity(entity->GetId()) && "Failed to add entity to scene properly!");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::NameStatic());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->AddEntityComponent(component);
        component = entity->GetComponent(component->Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::ComponentPtr component_second = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::NameStatic());
        entity->AddEntityComponent(component_second);
        component = entity->GetComponent(component_second->Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");
        entity->RemoveEntityComponent(component_second);
        component_second.reset();

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


        Foundation::ScenePtr cloned_scene = scene->Clone("Test clone scene");
        assert (sceneManager->HasScene("Test clone scene"));
        assert (cloned_scene->HasEntity(entity->GetId()) && "Failed to clone a scene");

        Foundation::EntityPtr cloned_entity = entity->Clone("Test clone scene");
        component = cloned_entity->GetComponent(component->Name());
        assert (component.get() != 0 && "Failed to clone an entity.");

        Foundation::EntityPtr cloned_entity2 = cloned_scene->GetEntity(cloned_entity->GetId());
        Foundation::EntityPtr entity2 = cloned_scene->CreateEntity(0);
        assert (*cloned_entity2.get() == *cloned_entity.get() && "EntityInterface operator== failed");
        assert (*entity.get() != *entity2.get() && "EntityInterface operator!= failed");
        assert (*cloned_entity.get() < *entity2.get() && "EntityInterface operator< failed");


        Foundation::ScenePtr scene2 = sceneManager->GetScene("test_scene");
        assert(*scene.get() == *scene2.get() && "SceneInterface operator== failed");
        assert(*scene.get() != *cloned_scene.get() && "SceneInterface operator!= failed");
        assert(*cloned_scene.get() < *scene.get() && "SceneInterface operator< failed");

        Foundation::SceneManager::iterator it = sceneManager->Begin();
        Foundation::SceneManager::iterator it_copy = it;
        int test_scenes = 0;
        for ( ; it_copy != sceneManager->End() ; ++it_copy)
        {
            if ((*it_copy)->Name() == "test_scene")
                test_scenes++;

            if ((*it_copy)->Name() == "Test clone scene")
                test_scenes++;
        }
        assert (test_scenes == 2 && "Scene iterator could not find all the scenes!");

        const Foundation::SceneManagerServiceInterface *const_sceneManager = const_cast<const Foundation::SceneManagerServiceInterface*>(sceneManager);
        Foundation::SceneManager::const_iterator c_it = const_sceneManager->Begin();
        Foundation::SceneManager::const_iterator c_it_copy = c_it;
        test_scenes = 0;
        for ( ; c_it_copy != const_sceneManager->End() ; ++c_it_copy)
        {
            if ((*c_it_copy)->Name() == "test_scene")
                test_scenes++;

            if ((*c_it_copy)->Name() == "Test clone scene")
                test_scenes++;
        }
        assert (test_scenes == 2 && "Const scene iterator could not find all the scenes!");



        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());


        TestEvent test_event;
        test_event.test_value_ = 12345;
        framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("Test"), 0, NULL);
        framework_->GetEventManager()->SendEvent(framework_->GetEventManager()->QueryEventCategory("Test"), 1, &test_event);

        framework_->Exit();
        assert (framework_->IsExiting());

        LogInfo("");
    }
}

