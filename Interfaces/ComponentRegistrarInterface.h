// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ComponentRegistrarInterface_h
#define incl_Interfaces_ComponentRegistrarInterface_h

#include <boost/shared_ptr.hpp>

namespace Foundation
{
    class Framework;

    class ComponentRegistrarInterface
    {
    public:
        ComponentRegistrarInterface()  {}
        virtual ~ComponentRegistrarInterface() {}
        
        virtual void Register(Framework *framework) = 0;

        virtual void Unregister(Framework *framework) = 0;
    };

    typedef boost::shared_ptr<ComponentRegistrarInterface> ComponentRegistrarInterfacePtr;
}

#endif
