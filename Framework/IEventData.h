// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_IEventData_h
#define incl_Interfaces_IEventData_h

#include <boost/shared_ptr.hpp>

/// Base class for all data that is passed as a parameter of a system-wide event.
class IEventData
{
public:
    IEventData()  {}
    virtual ~IEventData() {}
};

typedef boost::shared_ptr<IEventData> EventDataPtr;

#endif
