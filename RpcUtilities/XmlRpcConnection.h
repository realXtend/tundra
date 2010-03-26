// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RpcUtilities_XmlRpcConnection_h
#define incl_RPCUtilities_XmlRpcConnection_h

#include <xmlrpc.h>

/**
 * Represents a XMLRPC connection. You can do multiple XMLRPC requests/replies using the same connection.
 * @note use class throught XMLRPCEPI-class. 
 */
class XmlRpcConnection
{
public:
	
	/** 
	 * Overrided constructor which creates url where xml-data is send. 
	 * @param address is server address.
	 * @param port is server port.
	 */
	XmlRpcConnection(const std::string& address, const std::string& port);

	/** 
	 * Overrided constructor which creates url where xml-data is send. 
	 * @param url full url of xmlrpc request
	 * @param port is server port.
	 */
	XmlRpcConnection(const std::string& url);
	
	/**
	 * Sets server address. 
	 * @param address is server address. 
	 * @param port is server port.
	 */
	void SetServerAddress(const std::string& address, const std::string& port);

	void SetServer(const std::string& address) { strUrl_ = address; }
	
	/**
	 * Sends the XMLRPC request data (pure xml) over to the server.
	 * @param data is pure xml which is constructed in @p XMLRPCCall -class
	 * @return request object.
	 * @throw XMLRPCException is send failed for some reason.
	 **/
	XMLRPC_REQUEST Send(const char* data);  

private:
	std::string strUrl_;
};

#endif
