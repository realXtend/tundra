// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CoreStdIncludes.h"

#include "XmlRpcEpi.h"
#include "XmlRpcConnection.h"
#include "XmlRpcCall.h"

#include <xmlrpc.h>

XmlRpcEpi::XmlRpcEpi() 
: callMethod_(""), call_(0), connection_(0)
{
}

XmlRpcEpi::XmlRpcEpi(const std::string& method) 
: callMethod_(""), call_(0), connection_(0)
{
    CreateCall(method);
}

XmlRpcEpi::XmlRpcEpi(const std::string& method, const std::string& address, const std::string& port) 
: callMethod_(""), call_(0), connection_(0)
{
    Connect(address, port);
    CreateCall(method);
}

XmlRpcEpi::XmlRpcEpi(const std::string& method, const std::string& url) 
: callMethod_(""), call_(0), connection_(0)
{
    Connect(url);
    CreateCall(method);
}

XmlRpcEpi::~XmlRpcEpi()
{
    delete connection_;
    delete call_;
}

void XmlRpcEpi::Connect(const std::string& address, const std::string& port) 
{
    if ( connection_ == 0)
		connection_ = new XmlRpcConnection(address, port);
    else
        connection_->SetServerAddress(address, port);
    
}

void XmlRpcEpi::Connect(const std::string& url) 
{
    if ( connection_ == 0)
		connection_ = new XmlRpcConnection(url);
    else
        connection_->SetServer(url);
    
}

XmlRpcCall *XmlRpcEpi::GetXMLRPCCall()
{
    return call_;
}

void XmlRpcEpi::CreateCall(const std::string& method) 
{
    delete call_;
    call_ = 0;
    
    if (method != "")
        callMethod_ = method;

    if (callMethod_.size() > 0)
        call_ = new XmlRpcCall(callMethod_);
    else
        throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcEpi::CreateCall() method name was invalid"));
}

void XmlRpcEpi::Send()
{
    if (call_ == 0)
       throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcEpi::Send() Call object was zero pointer"));
    else if (connection_ == 0)
       throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcEpi::Send() Connection object was zero pointer"));
    
    // We now own xmlData, remember to deallocate using free();
    char *pXmlData = XMLRPC_REQUEST_ToXML(call_->GetRequest(), 0);
    if (pXmlData == 0)
        throw XmlRpcException(std::string("XmlRpcEpi exception in XmlRpcEpi::Send() xml data was zero pointer"));

    // If there exist old reply clear it out.
    if (call_->GetReply() != 0)
    {
        ///\todo Weak responsibilities.. XMLRPCEPI is freeing XMLRPCCall's data. Refactor pending. -jj.
        XMLRPC_RequestFree(call_->GetReply(),1);
        call_->SetReply(0);
    }

    try
    {
        call_->SetReply(connection_->Send(pXmlData));
    }
    catch(XmlRpcException& ex)
    {
        // Free xmlData
        XMLRPC_Free(pXmlData);
        pXmlData = 0;
        throw ex;
    }
    // Free xmlData
    XMLRPC_Free(pXmlData);
    pXmlData = 0;
}

void XmlRpcEpi::AddStringToArray(const std::string& name, const char *sstr)
{
    if (call_ != 0)
        call_->AddStringToArray(name, sstr);
}

void XmlRpcEpi::ClearCall()
{
    delete call_;
    call_ = 0;
}

bool XmlRpcEpi::HasReply(const char* name) const
{
    assert(name && strlen(name) > 0);

    if (call_ == 0)
        return false;
    XMLRPC_VALUE result = XMLRPC_RequestGetData(call_->GetReply());
    if (!result)
        return false;
    
    XMLRPC_VALUE resultValue = XMLRPC_VectorGetValueWithID(result, name);
    if (!resultValue)
        return false;
    else
        return true;
}
