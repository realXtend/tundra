
#include "StableHeaders.h"
#include "Entity.h"
#include "Framework.h"


namespace Foundation
{
    Core::uint Entity::smGID = 0;

    Entity::Entity(Framework *framework) : mFramework(framework), mId(smGID)
   {
       assert (framework != NULL);
       smGID = (smGID + 1) % static_cast<Core::uint>(-1);
   }

   Entity::~Entity()
   {
   }

   void Entity::addEntityComponent(const ComponentInterfacePtr &component)
   {
       mComponents.push_back(component);
       component->registerDesiredChanges(mFramework->getChangeManager(), getId());
   }

   void Entity::removeEntityComponent(const ComponentInterfacePtr &component)
   {
       ComponentVector::iterator iter = std::find(mComponents.begin(), mComponents.end(), component);
        mComponents.erase(iter);
   }
}
