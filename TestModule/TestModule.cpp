// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "TestModule.h"
#include "EC_Dummy.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"



namespace Test
{
    TestModule::TestModule() : ModuleInterface_Impl(Foundation::Module::Type_Test)
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

        LOG("Module " + Name() + " loaded.");
    }

    // virtual
    void TestModule::Unload()
    {
        LOG("Module " + Name() + " unloaded.");
    }

    // virtual
    void TestModule::Initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        framework_ = framework;
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Test, &test_service_);
        
        LOG("Module " + Name() + " initialized.");
    }

    // virtual 
    void TestModule::Uninitialize(Foundation::Framework *framework)
    {
        framework_->GetServiceManager()->UnregisterService(&test_service_);

        assert(framework_ != NULL);
        framework_ = NULL;
        
        LOG("Module " + Name() + " uninitialized.");
    }

    // virtual
    void TestModule::Update()
    {
        // create new entity
        LOG("Constructing entity with component: " + Test::EC_Dummy::Name() + ".");

        Foundation::EntityPtr entity = framework_->GetEntityManager()->createEntity();
        assert (entity.get() != 0 && "Failed to create entity.");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::Name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->addEntityComponent(component);
        component = entity->getComponent(component->_Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());

        framework_->Exit();
        assert (framework_->IsExiting());
    }
}

