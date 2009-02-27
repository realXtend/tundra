// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TestSystem.h"
#include "EC_Dummy.h"
#include "EC_Geometry.h"
#include "EC_OgreEntity.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"

//namespace Test
//{
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
        
        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void TestSystem::uninitialize(Foundation::Framework *framework)
    {
        assert(mFramework != NULL);
        mFramework = NULL;

        LOG("System " + name() + " uninitialized.");
    }

    void TestSystem::update()
    {
        // create new entity
        LOG(Test::EC_Dummy::name());

        //// One way to construct an entity
        ////
        //Core::StringVector components;
        //components.push_back(Geometry::EC_Dummy::name());
        //components.push_back(Geometry::EC_Geometry::name());
        //components.push_back(OgreRenderer::EC_OgreEntity::name());
        //Foundation::EntityPtr entity = mFramework->getEntityManager()->createEntity(components);

        // alternative way to construct an entity
        //
        Foundation::EntityPtr entity = mFramework->getEntityManager()->createEntity();
        entity->addEntityComponent(mFramework->getComponentManager()->createComponent(Test::EC_Dummy::name()));
        entity->addEntityComponent(mFramework->getComponentManager()->createComponent(Geometry::EC_Geometry::name()));

        entity = mFramework->getEntityManager()->createEntity();
        entity->addEntityComponent(mFramework->getComponentManager()->createComponent(Test::EC_Dummy::name()));
        entity->addEntityComponent(mFramework->getComponentManager()->createComponent(Geometry::EC_Geometry::name()));
        entity->addEntityComponent(mFramework->getComponentManager()->createComponent(OgreRenderer::EC_OgreEntity::name()));


        //// move pre-existing entities
        ////
        //// Case where packed data is predecoded
        ////
        //Core::entity_id_t id = entity->getId();
        //Foundation::Change change = mFramework->getChangeManager()->getGlobalChange(Geometry::EC_Geometry::name(), Geometry::EC_Geometry::Position);
        //Core::Vector3 newPos(1001);

        //mFramework->getChangeManager()->change(id, change, boost::any(newPos));
        //LOG("Testing sending new position: " + boost::lexical_cast<std::string>(newPos) + " for entity id: " + boost::lexical_cast<std::string>(id));

        //// Case where component does it's own decoding on the packet data
        ////
        //Core::entity_id_t id = entity->getId();
        //Foundation::ComponentInterface *component = mFramework->getComponentManager()->createComponent(packedData); // returns new component based on packed data
        //component->parsePackedData(packedData);
        //Foundation::Change change = mFramework->getChangeManager()->getGlobalChange(Geometry::EC_Geometry::name(), Geometry::EC_Geometry::All); 
        //mFramework->getChangeManager()->change(id, change, boost::any(component));
    }

    POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
       POCO_EXPORT_CLASS(TestSystem)
    POCO_END_MANIFEST

//}

