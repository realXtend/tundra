
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
class XMLRPCException : public std::exception
{
public:
    ///\todo Add the originating XMLRPCEPI object.
    explicit XMLRPCException(const std::string &str)
    :strMessage_(str)
    {
    }
    
    const char *what () const throw () { return strMessage_.c_str(); }

    /**
     * Sets error message. 
     * @param message is string which contains error message.
     */
    void SetMessage(const std::string& message) { strMessage_ = message; }

private:

    std::string strMessage_;
};
#endif 
