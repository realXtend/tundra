
// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCException_h
#define incl_OpenSimProtocolModule_XMLRPCException_h

#include "CoreException.h"
#include <string>
class XMLRPCException : public Core::Exception
{
public:
    XMLRPCException() : Core::Exception(), strMessage_("") {}
    virtual ~XMLRPCException() {}

    XMLRPCException(const std::string& message ) : Core::Exception(), strMessage_(message) {}

    std::string GetMessage() const { return strMessage_; }
    void SetMessage(const std::string& message) { strMessage_ = message; }

private:

    std::string strMessage_;

};
#endif 