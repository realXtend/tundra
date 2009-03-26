// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentFactoryInterface_h
#define incl_Interfaces_ComponentFactoryInterface_h

namespace Foundation
{
    class ComponentInterface_Abstract;

    class ComponentFactoryInterface
    {
    public:
        ComponentFactoryInterface() {}
        virtual ~ComponentFactoryInterface() {}

        virtual boost::shared_ptr<ComponentInterface_Abstract> operator()() = 0;
        virtual boost::shared_ptr<ComponentInterface_Abstract> operator()(const boost::shared_ptr<ComponentInterface_Abstract> &) = 0;
    };
}

#endif
