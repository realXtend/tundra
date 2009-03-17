// For conditions of distribution and use, see copyright notice in license.txt
#include <sstream>
#include <cstring>

#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/Net/HostEntry.h"
#include "Poco/Net/DNS.h"

#include "PocoXMLRPC.h"
#include "curl/curl.h"

using namespace std;

using Poco::XML::Document;
using Poco::XML::Element;
using Poco::XML::Text;
using Poco::XML::AutoPtr;
using Poco::XML::DOMWriter;
using Poco::XML::XMLWriter;


PocoXMLRPCConnection::PocoXMLRPCConnection(const char *address, int port)
:socket(Poco::Net::SocketAddress(address, port))
{
	
}

/// The writer callback function used by cURL.
size_t WriteCallback(char *data, size_t size, size_t nmemb, std::vector<char> *buffer)  
{  
	if (buffer)
	{
		buffer->insert(buffer->end(), data, data + size * nmemb);
		return size * nmemb;
	}
	else
		return 0;
}

/// Blocks to wait for 100-Continue HTTP reply.
/// Generates a query structure to call the given method using XMLRPC. The query will have an empty vector argument.
boost::shared_ptr<PocoXMLRPCCall> PocoXMLRPCConnection::StartXMLRPCCall(const char *methodName)
{
	assert(methodName && strlen(methodName) > 0);
	boost::shared_ptr<PocoXMLRPCCall> call = boost::shared_ptr<PocoXMLRPCCall>(new PocoXMLRPCCall);
	
	// create a new request object
	call->request = XMLRPC_RequestNew();
	assert(call->request);

	// Set the method name and tell it we are making a request
	XMLRPC_RequestSetMethodName(call->request, methodName);
	XMLRPC_RequestSetRequestType(call->request, xmlrpc_request_call);

	/* tell it to write out xml-rpc (default). options are: 
	* xmlrpc_version_1_0        // xmlrpc 1.0
	* xmlrpc_version_simple		 // simpleRPC
	* xmlrpc_version_soap_1_1	 // soap 1.1
	*/
	memset(&call->output, 0, sizeof(call->output));
	call->output.version = xmlrpc_version_1_0;
	XMLRPC_RequestSetOutputOptions(call->request, &call->output);

	// Create a parameter list vector
	call->xParamList = XMLRPC_CreateVector(NULL, xmlrpc_vector_struct);
	assert(call->xParamList);
	XMLRPC_RequestSetData(call->request, call->xParamList);

	return call;
}

///\ todo Not used anymore(?) so delete?
/// Blocks to wait for 100-Continue HTTP reply. \todo Replace this with curl or remove entirely?
/*bool PocoXMLRPCConnection::WaitForHTTP100Continue()
{
	std::vector<char> reply;
	reply.resize(512, 0);
	int received = 0;
	do {
		received = socket.receiveBytes(&reply[0], 510);
		std::cout << &reply[0];
		if (received != 0 && strstr(&reply[0], "100") == 0)
			return false;
	} while(received == 0);
	
	return true;
}*/

///\ todo Not used anymore(?) so delete?
/*void PocoXMLRPCConnection::ReadXMLRPCReply(PocoXMLRPCCall &call)
{
	std::stringstream replyString;

	time_t lastReceived = time(NULL);
	std::vector<char> reply;
	reply.resize(2048, 0);

	// Read in the TCP packets forming the reply until no more incoming.
	int received = 0;
	while(1) ///\todo Use a real HTTP library.
	{
		received = socket.receiveBytes(&reply[0], (int)reply.size() - 2);
		if (received > 0)
		{
			reply[received] = '\0';
			replyString << (char*)&reply[0];
			lastReceived = time(NULL);
		}
		if (time(NULL) - lastReceived > 5) ///\todo Not a real way to handle message end. Use a real HTTP library.
			break;
	}

	size_t xmlStart = replyString.str().find("<?xml");
	if (xmlStart == std::string::npos)
	{
		std::cout << "Invalid XMLRPC reply!" << std::endl;
		return;
	}

	// Convert the XML string to a XMLRPC reply structure.
	call.reply = XMLRPC_REQUEST_FromXML(replyString.str().c_str() + xmlStart, (int)(replyString.str().size() - xmlStart), 0);
	assert(call.reply);
}*/

bool PocoXMLRPCConnection::FinishXMLRPCCall(boost::shared_ptr<PocoXMLRPCCall> call)
{
	// serialize client request as XML
	char *outBuf = XMLRPC_REQUEST_ToXML(call->request, 0); // We now own outBuf, remember to deallocate using free();
	if (!outBuf)
		return false;

	curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl = curl_easy_init();
	if (!curl)
		return false;

	CURLcode result;
	char curl_error_buffer[CURL_ERROR_SIZE];
	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;
	struct curl_slist *headers = NULL;
	std::vector<char> response_data;
	
	char *url = "192.168.1.144:9000"; ///\todo Use the real url and port.

	headers = curl_slist_append(headers, "Accept-Encoding: deflate, gzip");
	headers = curl_slist_append(headers, "Content-Type: text/xml"); ///\todo Gets overriden with the default value.
	headers = curl_slist_append(headers, "Expect: 100-continue");
	headers = curl_slist_append(headers, "");

	curl_formadd(&post, &last, 	CURLFORM_CONTENTHEADER, headers, CURLFORM_END);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &outBuf[0]);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(outBuf));
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_buffer);

	result = curl_easy_perform(curl);
	
	// Clean up and free memory.
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_formfree(post);

	if (result != CURLE_OK)
	{
		cout << "cURL error: " << curl_error_buffer << "." << endl;
		return false;
	}

	XMLRPC_Free(outBuf);

	///\todo Not sure about XMLRPC-EPI, does it really free all the allocated memory just by calling this? Docs say so,
	///      but samples tell otherwise.
	XMLRPC_RequestFree(call->request, 1);
	call->request = 0;
	call->xParamList = 0;
	
	// Convert the XML string to a XMLRPC reply structure.
	if (response_data.size() == 0)
		return false;
	
	call->reply = XMLRPC_REQUEST_FromXML(&response_data[0], (int)(response_data.size()), 0);
	assert(call->reply);

	return true;
}

PocoXMLRPCCall::PocoXMLRPCCall()
:request(0), xParamList(0), reply(0)
{
	memset(&output, 0, sizeof(output));
}


const char *PocoXMLRPCCall::GetReplyString(const char *name)
{
	assert(name && strlen(name) > 0);
	// I'm probably not understanding the value hierarchy here.. samples use //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?
	XMLRPC_VALUE val = XMLRPC_RequestGetData(reply); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(val);
	if (!val)
	{
		cout << "Error! XML reply did not contain any data! (Tried to retrieve string by ID " << name << endl;
		return "";
	}

	XMLRPC_VALUE xVal = XMLRPC_VectorGetValueWithID(val, name);
	if (xVal)
		return XMLRPC_GetValueString(xVal);
	else
	{
		cout << "Warning! XML reply does not contain a string by ID " << name << endl;
		return "";
	}
}

uint32_t PocoXMLRPCCall::GetReplyInt(const char *name)
{
	assert(name && strlen(name) > 0);
	// I'm probably not understanding the value hierarchy here.. samples use //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	// but it seems to walk into the first element of the vector, after which GetValueWithID doesn't find the correct sibling. wtf?
	XMLRPC_VALUE val = XMLRPC_RequestGetData(reply); //XMLRPC_VectorRewind(XMLRPC_RequestGetData(request));
	assert(val);
	if (!val)
	{
		cout << "Error! XML reply did not contain any data! (Tried to retrieve int by ID " << name << endl;
		return 0;
	}
	XMLRPC_VALUE xVal = XMLRPC_VectorGetValueWithID(val, name);
	if (xVal)
		return (uint32_t)XMLRPC_GetValueInt(xVal);
	else
	{
		cout << "Warning! XML reply does not contain an int by ID " << name << endl;
		return 0;
	}
}

void PocoXMLRPCCall::AddIntMember(const char *name, int value)
{
	XMLRPC_VectorAppendInt(xParamList, name, value);
}

void PocoXMLRPCCall::AddStringMember(const char *name, const char *value)
{
	XMLRPC_VectorAppendString(xParamList, name, value, 0);
}

PocoXMLRPCCall::StringArray PocoXMLRPCCall::CreateStringArray(const char *name)
{
	XMLRPC_VALUE vec = XMLRPC_CreateVector(name, xmlrpc_vector_array);
	XMLRPC_AddValueToVector(xParamList, vec);

	return vec;
}

void PocoXMLRPCCall::AddStringToArray(StringArray xmlArray, const char *str)
{
	XMLRPC_VectorAppendString(xmlArray, NULL, str, 0);
}
