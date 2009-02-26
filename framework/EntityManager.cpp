#include "StableHeaders.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Framework.h"
#include "ComponentManager.h"

namespace Foundation
{
    EntityPtr EntityManager::createEntity(const Core::StringVector &components)
    {
        EntityPtr entity = EntityPtr(new Entity(mFramework));
        for (size_t i=0 ; i<components.size() ; ++i)
        {
            entity->addEntityComponent(mFramework->getComponentManager()->createComponent(components[i]));
        }

        //! \todo figure out entity id
        mEntities[0] = entity;

        return entity;
    }

    EntityPtr EntityManager::createEntity()
    {
        Core::StringVector empty;
        return createEntity(empty);
    }
    
    EntityPtr EntityManager::getEntity(Core::entity_id_t id)
    {
        EntityPtr entity = EntityPtr(new Entity(mFramework));
        return entity; // fixme 
    } 
    
}

