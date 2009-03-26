// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentInterface_h
#define incl_Interfaces_ComponentInterface_h

#include "ComponentFactoryInterface.h"
#include "ComponentRegistrarInterface.h"

namespace Foundation
{
    class Framework;

    class MODULE_API ComponentInterface_Abstract
    {
    public:
        ComponentInterface_Abstract()  {}
        virtual ~ComponentInterface_Abstract() { }
        
        virtual void HandleNetworkData(std::string data) {}

        virtual const std::string &Name() const = 0;
    };

    class MODULE_API ComponentInterface_Impl : public ComponentInterface_Abstract
    {
        ComponentInterface_Impl();
    public:
        ComponentInterface_Impl(Foundation::Framework *framework) : ComponentInterface_Abstract(), framework_(framework) {}
        ComponentInterface_Impl(const ComponentInterface_Impl &rhs) : framework_(rhs.framework_) {}
        virtual ~ComponentInterface_Impl() { framework_->GetComponentManager()->RemoveExpiredComponents(); }
        
    private:
        const Foundation::Framework * const framework_;
    };
    typedef ComponentInterface_Impl ComponentInterface;
}

#endif

