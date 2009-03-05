// For conditions of distribution and use, see copyright notice in license.txt

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
    }

    void Entity::removeEntityComponent(const ComponentInterfacePtr &component)
    {
        ComponentVector::iterator iter = std::find(mComponents.begin(), mComponents.end(), component);
        mComponents.erase(iter);
    }

    ComponentInterfacePtr Entity::getComponent(const std::string &name) const
    {
        for (size_t i=0 ; i<mComponents.size() ; ++i)
            if (mComponents[i]->_Name() == name)
                return mComponents[i];

        return ComponentInterfacePtr();
    }
}
