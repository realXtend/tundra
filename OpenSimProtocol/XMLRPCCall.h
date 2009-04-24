// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCCall_h
#define incl_OpenSimProtocolModule_XMLRPCCall_h

#include <xmlrpc.h>
#include <string>
#include <map>

/// Represents a XMLRPC function call.
//typedef XMLRPC_VALUE StringArray;

class XMLRPCCall
{
public:
    XMLRPCCall(const std::string& method);
    XMLRPCCall();
    virtual ~XMLRPCCall();

    XMLRPC_REQUEST& GetRequest() { return request_;}
    XMLRPC_VALUE& GetParamList() { return paramList_; }
    STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS& GetOptions() { return output_; }
    XMLRPC_REQUEST GetReply() const { return reply_; }
    
    void SetCallMethod(const std::string& method) {  XMLRPC_RequestSetMethodName(request_, method.c_str()); }

    void SetReply(XMLRPC_REQUEST reply) { reply_ = reply; }

    /// Adds a given string to a StringArray parameter.
	void AddStringToArray(const std::string& name, const char* sstr);


protected:

    /// Generates a query structure to call the given method using XMLRPC. The query will have an empty vector argument.
    void Initialize(const std::string& method);
   
    /*
	XMLRPC_REQUEST request;
	XMLRPC_VALUE xParamList;
	STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS output;

	XMLRPC_REQUEST reply;
    */
	
    /// Adds a string "name=value" parameter to the function call.
	//void AddStringMember(const char *name, const char *value);

	//void AddIntMember(const char *name, int value);

	/// Adds a (null) string array as a parameter to the function call. Use AddStringToArray to append strings to the array.
	//StringArray CreateStringArray(const char *name);

private:

    XMLRPC_REQUEST request_;
    XMLRPC_VALUE paramList_;
    XMLRPC_REQUEST reply_;
    STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS output_;
    std::map<std::string, XMLRPC_VALUE> mapArrays_;

};
#endif
