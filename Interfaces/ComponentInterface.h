// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

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

        virtual const std::string &_name() = 0;
    };

    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::shared_ptr<ComponentInterface> ComponentPtr;
}

#endif

