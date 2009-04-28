// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenSimProtocolModule_XMLRPCCall_h
#define incl_OpenSimProtocolModule_XMLRPCCall_h

#include <xmlrpc.h>
#include <string>
#include <map>

/**
 * Represents a XMLRPC function call.
 * @note do not use this class directly use it through @p XMLRPCEPI-class.
 */

class XMLRPCCall
{
public:
    
    /**
     * Overrided constructor which creates call method which METHOD is set to given param.
     * @param method is method name. 
     */
    XMLRPCCall(const std::string& method);
    XMLRPCCall();
    ~XMLRPCCall();

    /**
     * Returns request object (reference). 
     * @note XMLRPC_REQUEST is actually pointer which contains reference counter, be careful. 
     */
    XMLRPC_REQUEST& GetRequest() { return request_;}
    
    /**
     * Returns paramlist in current call. 
     * @note XMLRPC_VALUE is actually pointer which contains reference counter, be careful. 
     */
    XMLRPC_VALUE& GetParamList() { return paramList_; }
    
    /**
     * Returns current call options. 
     * @note STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS is actually pointer which contains reference counter, be careful. 
     */
    STRUCT_XMLRPC_REQUEST_OUTPUT_OPTIONS& GetOptions() { return output_; }
    
    /**
     * Returns reply request. 
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
