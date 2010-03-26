// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RpcUtilities_XmlRpcCall_h
#define incl_RpcUtilities_XmlRpcCall_h

#include <xmlrpc.h>
#include <string>
#include <map>

/**
 * Represents a XMLRPC function call.
 * @note do not use this class directly use it through @p XMLRPCEPI-class.
 */	
class XmlRpcCall
{
public:
    
	/**
	 * Overrided constructor which creates call method which METHOD is set to given param.
	 * @param method is method name. 
	 */
	explicit XmlRpcCall(const std::string& method);
	XmlRpcCall();
	~XmlRpcCall();

	/**
	 * Returns request object (reference). 
	 */
	XMLRPC_REQUEST GetRequest() { return request_;}
    
	/**
	 * Returns paramlist in current call. 
	 */
	XMLRPC_VALUE GetParamList() { return paramList_; }
    
	/**
	 * Returns current call options. 
	 */
//    STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS &GetOptions() { return output_; }
    
	/**
	 * Returns reply request.
	 * \todo What is a 'reply request'? In XMLRPC, we send a request structure, and get back a reply structure. Is this
	 *       for getting the request structure before sending the request so that more variables can be added to it using
	 *       the C API? Or is this for getting the request structure after the request has been sent so that one can use 
	 *       the C API to read the reply structure? Apparently this can be used for both (?), so recommend renaming just to
	 *       GetXMLRPC_REQUEST() to reflect it's a getter for a generic purpose on the underlying C structure. -jj.
	 */
	XMLRPC_REQUEST GetReply() const { return reply_; }
    
	/**
	 * Sets a call method (type)
	 * @param method is name of method. 
	 */
	void SetCallMethod(const std::string& method) {  XMLRPC_RequestSetMethodName(request_, method.c_str()); }

	/**
	 * Sets reply. 
	 * @param reply is XMLRPC reply is respond which is got from server.
	 */
	void SetReply(XMLRPC_REQUEST reply) { reply_ = reply; }

	/**
	 * Adds a given string to a StringArray parameter.
	 * @param name is array name where string is added. 
	 * @param sstr is string which will be added.
	 */
	void AddStringToArray(const std::string& name, const char* sstr);


protected:

	/// Generates a query structure to call the given method using XMLRPC. The query will have an empty vector argument.
	void Initialize(const std::string& method);
   
private:

	XMLRPC_REQUEST request_;
	XMLRPC_VALUE paramList_;
	XMLRPC_REQUEST reply_;
	STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS output_;
	std::map<std::string, XMLRPC_VALUE> mapArrays_;
};

#endif
