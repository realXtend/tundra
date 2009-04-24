// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCConnection_h
#define incl_OpenSimProtocolModule_XMLRPCConnection_h

#include "XMLRPCException.h"
#include <string>
#include <xmlrpc.h>

/// Represents a XMLRPC connection. You can do multiple XMLRPC requests/replies using the same connection.

class XMLRPCConnection
{
public:
	
    /// Connects to the given address.
    XMLRPCConnection(const std::string& address, const std::string& port);
	
    void SetServerAddress(const std::string& address, const std::string& port) { strUrl_ = address + ":" + port; }
    void SetServer(const std::string& address) { strUrl_ = address; }

	
	/// Sends the XMLRPC request over to the server.
	XMLRPC_REQUEST Send(const char* data);  

  

private:
    std::string strUrl_;

};

#endif