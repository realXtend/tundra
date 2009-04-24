// For conditions of distribution and use, see copyright notice in license.txt

#include "XMLRPCEPI.h"
#include "XMLRPCConnection.h"
#include "XMLRPCCall.h"

#include <xmlrpc.h>

XMLRPCEPI::XMLRPCEPI() 
: strCallMethod_(""), pCall_(0), pConnection_(0)
{}

XMLRPCEPI::XMLRPCEPI(const std::string& method) 
: strCallMethod_(""), pCall_(0), pConnection_(0)
{
    CreateCall(method);
}

XMLRPCEPI::XMLRPCEPI(const std::string& method, const std::string& address, const std::string& port) 
: strCallMethod_(""), pCall_(0), pConnection_(0)
{
    Connect(address, port);
    CreateCall(method);
}

XMLRPCEPI::~XMLRPCEPI()
{
    delete pConnection_;
    delete pCall_;
}

void XMLRPCEPI::Connect(const std::string& address, const std::string& port) 
{
    if ( pConnection_ == 0)
        pConnection_ = new XMLRPCConnection(address, port);
    else
        pConnection_->SetServerAddress(address, port);
    
}

void XMLRPCEPI::CreateCall(const std::string& method) 
{
    if ( pCall_ != 0)
    {
        delete pCall_;
        pCall_ = 0;
    }
    
    if ( method != "" )
    {
        pCall_ = new XMLRPCCall(method);
        strCallMethod_ = method;
    }
    else if ( strCallMethod_ != "" )
        pCall_ = new XMLRPCCall(strCallMethod_);
    else
    {
        // todo PENDING throw exception.
    }
}

void XMLRPCEPI::Send()
{
    if ( pCall_ == 0 ) 
    {
        // todo PENDING throw exception.
    }
    else if ( pConnection_ == 0 )
    {
        // todo PENDING throw exception.
       
    }
    
    // We now own xmlData, remember to deallocate using free();

    char *pXmlData = XMLRPC_REQUEST_ToXML(pCall_->GetRequest(), 0); 
    if (pXmlData == 0)
    {
        //todo PENDING throw exception
    }

    
    pCall_->SetReply(pConnection_->Send(pXmlData));
    
    // Free xmlData
    XMLRPC_Free(pXmlData);
}

void XMLRPCEPI::AddStringToArray(const std::string& name, const char *sstr)
{
    if ( pCall_ != 0)
        pCall_->AddStringToArray(name, sstr);

}

void XMLRPCEPI::ClearCall()
{
    delete pCall_;
    pCall_ = 0;
}


