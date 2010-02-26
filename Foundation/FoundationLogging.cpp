// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Poco/Logger.h>

#include "ForwardDefines.h"

namespace Foundation
{
    //! Use root logging only in foundation classes.
    void RootLogFatal(const std::string &msg)
    {
        Poco::Logger::get("Foundation").fatal("Fatal: " + msg);
    }
    void RootLogCritical(const std::string &msg)
    {
        Poco::Logger::get("Foundation").critical("Critical: " + msg);
    }
    void RootLogError(const std::string &msg)
    {
        Poco::Logger::get("Foundation").error("Error: " + msg);
    }
    void RootLogWarning(const std::string &msg)
    {
        Poco::Logger::get("Foundation").warning("Warning: " + msg);
    }
    void RootLogNotice(const std::string &msg)
    {
        Poco::Logger::get("Foundation").notice("Notice: " + msg);
    }
    void RootLogInfo(const std::string &msg)
    {
        Poco::Logger::get("Foundation").information(msg);
    }
    void RootLogTrace(const std::string &msg)
    {
        Poco::Logger::get("Foundation").trace("Trace: " + msg);
    }
    void RootLogDebug(const std::string &msg)
    {
        Poco::Logger::get("Foundation").debug("Debug: " + msg);
    }
}
