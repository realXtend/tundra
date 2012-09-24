// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "XMPPModule.h"
#include "Client/Client.h"
#include "Client/AccountManager.h"

#include "Framework.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{
XMPPModule::XMPPModule() :
    IModule("XMPP"),
    account_manager_(0)
{
}

XMPPModule::~XMPPModule()
{
    for(int i = 0; i < clients_.size(); i++)
    {
        SAFE_DELETE(clients_[i]);
    }
    clients_.clear();
    SAFE_DELETE(account_manager_);
}

void XMPPModule::Load()
{
    framework_->RegisterDynamicObject("xmpp", this);
}

void XMPPModule::Initialize()
{
    account_manager_ = new AccountManager(framework_);
    account_manager_->LoadFromFile();
}

void XMPPModule::Uninitialize()
{
}

void XMPPModule::Update(f64 frametime)
{
    for(int i = 0; i < clients_.size(); i++)
    {
        clients_[i]->Update(frametime);
    }
}

QObject* XMPPModule::NewClient()
{
    Client *client = new Client(framework_);
    clients_.append(client);
    return dynamic_cast<QObject*>(client);
}

QObject* XMPPModule::GetAccountManager()
{
    if(account_manager_)
        return dynamic_cast<QObject*>(account_manager_);
    return 0;
}

void XMPPModule::HandleClientDisconnect(Client *client)
{
    //SAFE_DELETE(client);
}

}// end of namespace: XMPP

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new XMPP::XMPPModule();
        fw->RegisterModule(module);
    }
}
