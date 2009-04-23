// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Rex_PocoXMLRPC_h
#define incl_Rex_PocoXMLRPC_h

#include <iostream>

#include <boost/shared_ptr.hpp>

#include "Poco/Net/StreamSocket.h" 
#include "Poco/Net/SocketStream.h" 
#include "Poco/Net/SocketAddress.h" 
#include "Poco/StreamCopier.h" 
#include "Poco/Path.h" 
#include "Poco/Exception.h" 
#include "Poco/DOM/DOMBuilder.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/AutoPtr.h"

#include "RexTypes.h"
#include <xmlrpc.h>

/// Represents a XMLRPC function call.
class PocoXMLRPCCall
{
public:
	PocoXMLRPCCall();

	/// Adds a string "name=value" parameter to the function call.
	void AddStringMember(const char *name, const char *value);

	void AddIntMember(const char *name, int value);

	typedef XMLRPC_VALUE StringArray;

	/// Adds a (null) string array as a parameter to the function call. Use AddStringToArray to append strings to the array.
	StringArray CreateStringArray(const char *name);

	/// Adds a given string to a StringArray parameter.
	void AddStringToArray(StringArray array, const char *str);

	const char *GetReplyString(const char *name);

	uint32_t GetReplyInt(const char *name);

	XMLRPC_REQUEST request;
	XMLRPC_VALUE xParamList;
	STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS output;
	XMLRPC_REQUEST reply;	
};

/// Represents a XMLRPC connection. You can do multiple XMLRPC requests/replies using the same connection.
class PocoXMLRPCConnection
{
public:
	/// Connects to the given address.
	PocoXMLRPCConnection(const char *address, int port);
	
	/// Writer callback for cURL.
	//size_t WriteCallback(char *data, size_t size, size_t nmemb, std::vector<char> *buffer);
	
	/// Starts a new request. To actually send the request over, call FinishXMLRPCCall.
	/// @return A structure that represents the empty request. Call its member functions to fill in the function call
	///         parameters before calling FinishXMLRPCCall to submit the actual request.
	boost::shared_ptr<PocoXMLRPCCall> StartXMLRPCCall(const char *methodName);

	/// Sends the XMLRPC request over to the server.
	bool FinishXMLRPCCall(boost::shared_ptr<PocoXMLRPCCall> call);

private:
	Poco::Net::StreamSocket socket;
	
	std::string url_;

	/// @return True if the server sent back HTTP 100 code.
//	bool WaitForHTTP100Continue();

	void ReadXMLRPCReply(PocoXMLRPCCall &call);

};

#endif
