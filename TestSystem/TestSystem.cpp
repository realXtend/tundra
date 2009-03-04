// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"



#include "TestSystem.h"
#include "EC_Dummy.h"
#include "EC_Geometry.h"
#include "EC_OgreEntity.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"



namespace Test
{
    TestSystem::TestSystem() : ModuleInterface_Impl(Foundation::Module::Type_Test)
    {
    }

    TestSystem::~TestSystem()
    {
    }

    // virtual
    void TestSystem::load()
    {
        using namespace Test;
        DECLARE_MODULE_EC(EC_Dummy);

        LOG("System " + name() + " loaded.");
    }

    // virtual
    void TestSystem::unload()
    {
        LOG("System " + name() + " unloaded.");
    }

    // virtual
    void TestSystem::initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        mFramework = framework;
        mFramework->getServiceManager()->registerService(Foundation::Service::ST_Test, this);
        
        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void TestSystem::uninitialize(Foundation::Framework *framework)
    {
        mFramework->getServiceManager()->unregisterService(this);

        assert(mFramework != NULL);
        mFramework = NULL;
        
        LOG("System " + name() + " uninitialized.");
    }

    void TestSystem::update()
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
   POCO_EXPORT_CLASS(TestSystem)
POCO_END_MANIFEST



