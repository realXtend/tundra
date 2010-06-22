// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "XmlRpcCall.h"

XmlRpcCall::XmlRpcCall()
:request_(0),paramList_(0), reply_(0)
{
	memset(&output_, 0, sizeof(output_));
}

XmlRpcCall::XmlRpcCall(const std::string& method)
: request_(0), paramList_(0), reply_(0)
{
    memset(&output_, 0, sizeof(output_));
    Initialize(method);
}

XmlRpcCall::~XmlRpcCall()
{
    
    XMLRPC_RequestFree(request_, 1);
    request_ = 0;
    XMLRPC_RequestFree(reply_,1);
    reply_ = 0;
    paramList_ = 0;
  
}

void XmlRpcCall::Initialize(const std::string& method)
{
    assert(method != "");
	
  	// Create a new request object
	
    request_ = XMLRPC_RequestNew();

    // Assure that request was done correctly 
    
    assert(request_);

	// Set the method name and tell it we are making a request
	
    XMLRPC_RequestSetMethodName(request_, method.c_str());
	XMLRPC_RequestSetRequestType(request_, xmlrpc_request_call);

    memset(&output_, 0, sizeof(output_));
	
    // Tell it to write out xml-rpc (default). 
	output_.version = xmlrpc_version_1_0;
    
    // Other options are: 
	// xmlrpc_version_1_0        // xmlrpc 1.0
	// xmlrpc_version_simple     // simpleRPC
	// xmlrpc_version_soap_1_1	 // soap 1.1


    XMLRPC_RequestSetOutputOptions(request_, &output_);

	// Create a parameter list vector
	
    paramList_ = XMLRPC_CreateVector(0, xmlrpc_vector_struct);
	assert(paramList_);
	XMLRPC_RequestSetData(request_, paramList_);

}


void XmlRpcCall::AddStringToArray(const std::string& name, const char *sstr)
{
    // Does this kind array exist in current xmlrpc-call?
    std::map<std::string, XMLRPC_VALUE>::iterator iter = mapArrays_.find(name);
    
    if ( iter != mapArrays_.end())
    {
        //There exist already given array. Add string there. 
        XMLRPC_VectorAppendString(iter->second, 0, sstr, 0);
    }
    else
    {
        // Create new array into xmlrpc-call.
        mapArrays_[name] = XMLRPC_CreateVector(name.c_str(), xmlrpc_vector_array);
        
        // Note here paramList owns created vector (assumption) and paramList is owned by request_ object. 
	    XMLRPC_AddValueToVector(paramList_, mapArrays_[name]);
        
        // Add string there.
        XMLRPC_VectorAppendString(mapArrays_[name], 0, sstr, 0);

    }

}