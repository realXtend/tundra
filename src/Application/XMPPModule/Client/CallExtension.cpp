// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CallExtension.h"
#include "Client.h"
#include "XMPPModule.h"
#include "UserItem.h"

#include "LoggingFunctions.h"

#include "qxmpp/QXmppUtils.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{

QString CallExtension::extension_name_ = "Call";

CallExtension::CallExtension() :
    Extension(extension_name_),
    qxmpp_call_manager_(0)
{
}

CallExtension::~CallExtension()
{
    QString call;
    foreach(call, calls_.keys())
    {
        delete calls_[call];
        calls_.remove(call);
    }
}

void CallExtension::Initialize(Client *client)
{
    qxmpp_call_manager_ = new QXmppCallManager();

    client_ = client;
    client_->GetQxmppClient()->addExtension(qxmpp_call_manager_);
    framework_ = client_->GetFramework();

    bool check;
    check = connect(qxmpp_call_manager_, SIGNAL(callReceived(QXmppCall*)), this, SLOT(HandleCallReceived(QXmppCall*)));
    Q_ASSERT(check);
}

void CallExtension::Update(f64 frametime)
{
    QString call;
    foreach(call, calls_.keys())
        calls_[call]->Update(frametime);
}

bool CallExtension::AcceptCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid) || calls_[peerJid]->CurrentState() != Call::RingingState)
        return false;

    return calls_[peerJid]->Accept();
}

// callType is ignored becouse videochannel is not implemented in QXmpp 0.3.0
bool CallExtension::CallUser(QString peerJid, QString peerResource, int callType)
{
    if(!client_ || !client_->GetUser(peerJid))
        return false;

    //UserItem* user_item = static_cast<UserItem*>(client_->getUser(peerJid));
    //if(!user_item->getCapabilities(peerResource).contains("voice-v1"))
    //    return false;

    QString full_jid = peerJid + "/" + peerResource;

    QXmppCall *qxmpp_call = qxmpp_call_manager_->call(full_jid);

    if(!qxmpp_call)
        return false;

    /// \todo Check if we miss a signal becouse QXmppCall signals are suscribed inside XMPP::Call constructor
    Call *call = new Call(framework_, qxmpp_call);

    bool check = connect(call, SIGNAL(StateChanged(Call::State)), this, SLOT(HandleCallStateChanged(Call::State)));
    Q_ASSERT(check);

    calls_.insert(peerJid, call);
    return true;
}

bool CallExtension::CallUser(QString peerJid, QString peerResource, QStringList callType)
{
    int flags = 0;

    if(callType.size() == 0)
    {
        flags ^= 1;
    }
    else
    {
        if(callType.contains("Voice", Qt::CaseInsensitive))
            flags ^= 1;
        if(callType.contains("Video", Qt::CaseInsensitive))
            flags ^= 2;
    }

    return CallUser(peerJid, peerResource, flags);
}

bool CallExtension::DisconnectCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid))
        return false;

    calls_[peerJid]->Hangup();
    return true;
}

bool CallExtension::SuspendCall(QString peerJid)
{
    if(!calls_.contains(peerJid))
        return false;

    return calls_[peerJid]->Suspend();
}

QString CallExtension::GetActiveCall()
{
    QString call;
    foreach(call, calls_.keys())
    {
        if(calls_[call]->CurrentState() == Call::ActiveState)
            return calls_[call]->PeerJid();
    }
    return "";
}

QStringList CallExtension::GetCalls() const
{
    return calls_.keys();
}

bool CallExtension::SetActiveCall(QString peerJid)
{
    if(!calls_.keys().contains(peerJid))
        return false;

    if(calls_[peerJid]->CurrentState() != Call::SuspendedState)
        return false;

    SuspendCall(GetActiveCall());
    return calls_[peerJid]->Resume();
}

void CallExtension::HandleCallReceived(QXmppCall *qxmppCall)
{
    QString from_jid = jidToBareJid(qxmppCall->jid());

    LogDebug("XMPPModule: Incoming call from: " + from_jid.toStdString());

    Call *call = new Call(framework_, qxmppCall);
    calls_.insert(from_jid, call);

    emit CallIncoming(from_jid);
}

void CallExtension::HandleCallDisconnected(Call *call)
{
    emit CallDisconnected(call->PeerJid());
    calls_.remove(call->PeerJid());
    delete call;
}

void CallExtension::HandleCallStateChanged(Call::State state)
{
    Call *call = qobject_cast<Call*>(sender());
    if(!call)
        return;

    switch(state)
    {
    case Call::RingingState:
    case Call::ConnectingState:
        break;
    case Call::ActiveState:
        emit CallActive(call->PeerJid());
        break;
    case Call::SuspendedState:
        emit CallSuspended(call->PeerJid());
        break;
    case Call::DisconnectingState:
        break;
    case Call::FinishedState:
        HandleCallDisconnected(call);
        break;
    }
}

} // end of namespace: XMPP
