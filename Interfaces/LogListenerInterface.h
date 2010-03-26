// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_LogListenerInterface_h
#define incl_Interfaces_LogListenerInterface_h

namespace Foundation
{    
    //! abstract interface for log listener
    class LogListenerInterface
    {
    public:
        //! default constructor
        LogListenerInterface() {}

        //! destructor
        virtual ~LogListenerInterface() {}

        //! A very generic log message passing, only the message itself.
        //! Does not handle log levels or anything extra.
        virtual void LogMessage(const std::string &message) = 0;
    };

    typedef boost::shared_ptr<LogListenerInterface> LogListenerPtr;
}

#endif
