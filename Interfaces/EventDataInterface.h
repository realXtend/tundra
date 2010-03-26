// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_EventDataInterface_h
#define incl_Interfaces_EventDataInterface_h

#include <boost/shared_ptr.hpp>

namespace Foundation
{
    /// Base class for all data that is passed as a parameter of a system-wide event.
    class EventDataInterface
    {
    public:
        EventDataInterface()  {}
        virtual ~EventDataInterface() {}
    };
    
    typedef boost::shared_ptr<EventDataInterface> EventDataPtr;
}

#endif

