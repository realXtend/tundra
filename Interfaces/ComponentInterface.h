// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"

namespace Foundation
{
    class Framework;

    class MODULE_API ComponentInterface
    {
    public:
        ComponentInterface()  {}
        virtual ~ComponentInterface() { /*framework_->GetComponentManager()->RemoveComponent(Name(), this);*/ }
        
        virtual void HandleNetworkData(std::string data) {}

        virtual const std::string &Name() const { static std::string empty; return empty; };//= 0;
    private:
        Foundation::Framework *framework_;
    };
}

#endif

