// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ServiceInterface_h
#define incl_Interfaces_ServiceInterface_h

#include "CoreDefines.h"

namespace Foundation
{
    //! Common interface for all services
    /*!
        \ingroup Foundation_group
        \ingroup Services_group
    */
    class MODULE_API ServiceInterface
    {
    public:
        ServiceInterface() {}
        virtual ~ServiceInterface() {}
    };

    typedef boost::shared_ptr<ServiceInterface> ServicePtr;
    typedef boost::weak_ptr<ServiceInterface> ServiceWeakPtr;
}

#endif
