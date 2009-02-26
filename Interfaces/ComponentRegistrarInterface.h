
#ifndef __incl_Interfaces_ComponentRegistrarInterface_h__
#define __incl_Interfaces_ComponentRegistrarInterface_h__

namespace Foundation
{
    class Framework;

    class ComponentRegistrarInterface
    {
    public:
        ComponentRegistrarInterface()  {}
        virtual ~ComponentRegistrarInterface() {}
        
        virtual void _register(Framework *framework) = 0 {}

        virtual void _unregister(Framework *framework) = 0 {}
    };

    typedef boost::shared_ptr<ComponentRegistrarInterface> ComponentRegistrarInterfacePtr;
}

#endif
