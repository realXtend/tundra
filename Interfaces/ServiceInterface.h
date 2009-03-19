// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ServiceInterface_h
#define incl_Interfaces_ServiceInterface_h

namespace Foundation
{
    class ServiceInterface
    {
    public:
        ServiceInterface() {}
        virtual ~ServiceInterface() {}
    };

    typedef boost::shared_ptr<ServiceInterface> ServicePtr;
}

#endif

