// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentFactoryInterface_h
#define incl_Interfaces_ComponentFactoryInterface_h

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
