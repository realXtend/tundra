#ifndef __incl_Interfaces_ComponentFactoryInterface_h__
#define __incl_Interfaces_ComponentFactoryInterface_h__

namespace Foundation
{
    class ComponentInterface;

    class ComponentFactoryInterface
    {
    public:
        ComponentFactoryInterface() {}
        virtual ~ComponentFactoryInterface() {}

        virtual boost::shared_ptr<ComponentInterface> operator()() = 0;
        virtual boost::shared_ptr<ComponentInterface> operator()(const boost::shared_ptr<ComponentInterface> &) = 0;
    };
}

#endif
