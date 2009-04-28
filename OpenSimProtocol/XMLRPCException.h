
// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCException_h
#define incl_OpenSimProtocolModule_XMLRPCException_h

#include <cstdlib>
#include <cstring>
#include <string>

#include "CoreException.h"


/**
 * XMLRPCException is exception type which is throwed in cases if you use XMLRPC-epi interface and problem arises. 
 * @see Core::Exception
 */
class XMLRPCException : public Core::Exception
{
public:
    XMLRPCException() : Core::Exception(), strMessage_("") {}
    virtual ~XMLRPCException() throw () {} 

    /**
     * Override constructor. 
     * @param message is error message. 
     */
    XMLRPCException(const std::string& message ) : Core::Exception(), strMessage_(message) {}
    
    /**
     * Returns error message for caller. 
     * @return message which contains error. 
     */
    std::string GetMessage() const { return strMessage_; }
    
    /**
     * Sets error message. 
     * @param message is string which contains error message.
     */
    void SetMessage(const std::string& message) { strMessage_ = message; }

private:

    std::string strMessage_;

};
#endif 
