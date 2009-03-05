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
    void TestModule::load()
    {
        using namespace Test;
        DECLARE_MODULE_EC(EC_Dummy);

        LOG("System " + name() + " loaded.");
    }

    // virtual
    void TestModule::unload()
    {
        LOG("System " + name() + " unloaded.");
    }

    // virtual
    void TestModule::initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        mFramework = framework;
        mFramework->getServiceManager()->registerService(Foundation::Service::ST_Test, this);
        
        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void TestModule::uninitialize(Foundation::Framework *framework)
    {
        mFramework->getServiceManager()->unregisterService(this);

        assert(mFramework != NULL);
        mFramework = NULL;
        
        LOG("System " + name() + " uninitialized.");
    }

    // virtual
    void TestModule::update()
    {
        // create new entity
        LOG("Constructing entity with component: " + Test::EC_Dummy::name() + ".");

        Foundation::EntityPtr entity = mFramework->getEntityManager()->createEntity();
        assert (entity.get() != 0 && "Failed to create entity.");

        Foundation::ComponentPtr component = mFramework->getComponentManager()->createComponent(Test::EC_Dummy::name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->addEntityComponent(component);
        component = entity->getComponent(component->_name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::TestServiceInterface *test_service = mFramework->getServiceManager()->getService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->test());

        mFramework->_exit();
        assert (mFramework->isExiting());
    }
}

using namespace Test;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TestModule)
POCO_END_MANIFEST
