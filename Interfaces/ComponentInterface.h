// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_ComponentInterface_h__
#define __incl_Interfaces_ComponentInterface_h__

#include "ForwardDefines.h"
#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"

namespace Foundation
{
    class ComponentInterface
    {
    public:
        ComponentInterface()  {}
        virtual ~ComponentInterface() {}
        
        virtual void handleNetworkData(std::string data) {}

        virtual void registerDesiredChanges(const ChangeManagerPtr&, Core::entity_id_t) {}

        virtual const std::string &_name() = 0;
    };

    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
}

#endif

