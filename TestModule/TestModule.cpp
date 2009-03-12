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

        Foundation::SceneServiceInterface *scene = framework_->GetService<Foundation::SceneServiceInterface>(Foundation::Service::ST_Scene);
        Foundation::EntityPtr entity = scene->CreateEntity();
        assert (entity.get() != 0 && "Failed to create entity.");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::Name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->AddEntityComponent(component);
        component = entity->GetComponent(component->_Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());

        framework_->Exit();
        assert (framework_->IsExiting());

        LogInfo("");
    }
}

