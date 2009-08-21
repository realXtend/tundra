// For conditions of distribution and use, see copyright notice in license.txt

#include "CoreStdIncludes.h"
#include "XMLRPCEPI.h"
#include "XMLRPCConnection.h"
#include "XMLRPCCall.h"

#include <xmlrpc.h>

XMLRPCEPI::XMLRPCEPI() 
: callMethod_(""), call_(0), connection_(0)
{}

XMLRPCEPI::XMLRPCEPI(const std::string& method) 
: callMethod_(""), call_(0), connection_(0)
{
    CreateCall(method);
}

XMLRPCEPI::XMLRPCEPI(const std::string& method, const std::string& address, const std::string& port) 
: callMethod_(""), call_(0), connection_(0)
{
    Connect(address, port);
    CreateCall(method);
}

XMLRPCEPI::~XMLRPCEPI()
{
    delete connection_;
    delete call_;
}

void XMLRPCEPI::Connect(const std::string& address, const std::string& port) 
{
    if ( connection_ == 0)
        connection_ = new XMLRPCConnection(address, port);
    else
        connection_->SetServerAddress(address, port);
    
}

XMLRPCCall *XMLRPCEPI::GetXMLRPCCall()
{
    return call_;
}

void XMLRPCEPI::CreateCall(const std::string& method) 
{
    delete call_;
    call_ = 0;
    
    if (method != "")
        callMethod_ = method;

    if (callMethod_.size() > 0)
        call_ = new XMLRPCCall(callMethod_);
    else
        throw XMLRPCException(std::string("XMLRPCEPI exception in XMLRPCEPI::CreateCall() method name was invalid"));
}

void XMLRPCEPI::Send()
{
    if (call_ == 0)
       throw XMLRPCException(std::string("XMLRPCEPI exception in XMLRPCEPI::Send() Call object was zero pointer"));
    else if (connection_ == 0)
       throw XMLRPCException(std::string("XMLRPCEPI exception in XMLRPCEPI::Send() Connection object was zero pointer"));
    
    // We now own xmlData, remember to deallocate using free();
    char *pXmlData = XMLRPC_REQUEST_ToXML(call_->GetRequest(), 0);
    if (pXmlData == 0)
        throw XMLRPCException(std::string("XMLRPCEPI exception in XMLRPCEPI::Send() xml data was zero pointer"));

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
    catch(XMLRPCException& ex)
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

void XMLRPCEPI::AddStringToArray(const std::string& name, const char *sstr)
{
    if (call_ != 0)
        call_->AddStringToArray(name, sstr);
}

void XMLRPCEPI::ClearCall()
{
    delete call_;
    call_ = 0;
}
