// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Server.h"
#include "TundraLogicModule.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "SyncManager.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "CoreStringUtils.h"
#include "TundraMessages.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"

#include "SceneAPI.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

#include <QtScript>
#include <QDomDocument>

#include <boost/program_options.hpp>

Q_DECLARE_METATYPE(UserConnection*);
Q_DECLARE_METATYPE(UserConnectedResponseData*);

// The following functions help register a custom QObject-derived class to a QScriptEngine.
// See http://lists.trolltech.com/qt-interest/2007-12/thread00158-0.html .
template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject);
}

template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{   
    qobject = qobject_cast<Tp>(value.toQObject());
}

template <typename Tp>
int qScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , Tp * = 0
#endif
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}


using namespace kNet;

namespace TundraLogic
{

Server::Server(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    current_port_(-1),
    current_protocol_("")
{
    tundraEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Tundra");
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");
    KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)));

    connect(kristalli, SIGNAL(ClientDisconnectedEvent(UserConnection *)), this, SLOT(HandleUserDisconnected(UserConnection *)));
}

Server::~Server()
{
}

void Server::Update(f64 frametime)
{
}

bool Server::Start(unsigned short port)
{
    if (!owner_->IsServer())
    {
        kNet::SocketTransportLayer transportLayer = owner_->GetKristalliModule()->defaultTransport;
        if (!owner_->GetKristalliModule()->StartServer(port, transportLayer))
        {
            TundraLogicModule::LogError("Failed to start server in port " + ToString<int>(port));
            return false;
        }

        // Store current port and protocol
        current_port_ = (int)port;
        current_protocol_ = transportLayer == kNet::SocketOverUDP ? "udp" : "tcp";

        // Create the default server scene
        /// \todo Should be not hard coded like this. Give some unique id (uuid perhaps) that could be returned to the client to make the corresponding named scene in client?
        Scene::ScenePtr scene = framework_->Scene()->CreateScene("TundraServer", true);
        framework_->Scene()->SetDefaultScene(scene);
        owner_->GetSyncManager()->RegisterToScene(scene);
        
        // Create an authoritative physics world
        Physics::PhysicsModule *physics = framework_->GetModule<Physics::PhysicsModule>();
        physics->CreatePhysicsWorldForScene(scene, false);
        
        /// \todo Hack - find better way and remove! Allow environment also on server by sending a fake connect event
        Events::TundraConnectedEventData event_data;
        event_data.user_id_ = 0;
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_CONNECTED, &event_data);
        
        emit ServerStarted();
    }
    
    return true;
}

void Server::Stop()
{
    if (!owner_->IsServer())
    {
        owner_->GetKristalliModule()->StopServer();
        framework_->Scene()->RemoveScene("TundraServer");
        
        emit ServerStopped();
    }
}

bool Server::IsRunning() const
{
    return owner_->IsServer();
}

bool Server::IsAboutToStart() const
{
    const boost::program_options::variables_map &programOptions = framework_->ProgramOptions();
    if (programOptions.count("startserver"))
        return true;
    return false;
}

int Server::GetPort() const
{
    return current_port_;
}

QString Server::GetProtocol() const
{
    return current_protocol_;
}

UserConnectionList Server::GetAuthenticatedUsers() const
{
    UserConnectionList ret;
    
    UserConnectionList& all = owner_->GetKristalliModule()->GetUserConnections();
    for (UserConnectionList::const_iterator iter = all.begin(); iter != all.end(); ++iter)
    {
        if ((*iter)->properties["authenticated"] == "true")
            ret.push_back(*iter);
    }
    
    return ret;
}

QVariantList Server::GetConnectionIDs() const
{
    QVariantList ret;
    
    UserConnectionList users = GetAuthenticatedUsers();
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        ret.push_back(QVariant((*iter)->userID));
    
    return ret;
}

UserConnection* Server::GetUserConnection(int connectionID) const
{
    UserConnectionList users = GetAuthenticatedUsers();
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if ((*iter)->userID == connectionID)
            return (*iter);
    }
    
    return 0;
}

UserConnectionList& Server::GetUserConnections() const
{
    return owner_->GetKristalliModule()->GetUserConnections();
}

UserConnection* Server::GetUserConnection(kNet::MessageConnection* source) const
{
    return owner_->GetKristalliModule()->GetUserConnection(source);
}

void Server::SetActionSender(UserConnection* user)
{
    actionsender_ = user;
}

UserConnection* Server::GetActionSender() const
{
    return actionsender_;
}

void Server::HandleKristalliEvent(event_id_t event_id, IEventData* data)
{
    if (event_id == KristalliProtocol::Events::NETMESSAGE_IN)
    {
        if (owner_->IsServer())
        {
            KristalliProtocol::Events::KristalliNetMessageIn* eventData = checked_static_cast<KristalliProtocol::Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
        }
    }
    if (event_id == KristalliProtocol::Events::USER_DISCONNECTED)
    {
        KristalliProtocol::Events::KristalliUserDisconnected* eventData = checked_static_cast<KristalliProtocol::Events::KristalliUserDisconnected*>(data);
        UserConnection* user = eventData->connection;
        if (user)
            HandleUserDisconnected(user);
    }
}

void Server::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
{
    if (!owner_->IsServer())
        return;
        
    // If we are server, only allow the login message from an unauthenticated user
    if (id != cLoginMessage)
    {
        UserConnection* user = GetUserConnection(source);
        if ((!user) || (user->properties["authenticated"] != "true"))
        {
            TundraLogicModule::LogWarning("Server: dropping message " + ToString(id) + " from unauthenticated user");
            //! \todo something more severe, like disconnecting the user
            return;
        }
    }
    
    switch (id)
    {
        // Server
    case cLoginMessage:
        {
            MsgLogin msg(data, numBytes);
            HandleLogin(source, msg);
        }
        break;
    }
}

void Server::HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg)
{
    UserConnection* user = GetUserConnection(source);
    if (!user)
    {
        TundraLogicModule::LogWarning("Login message from unknown user");
        return;
    }
    
    QDomDocument xml;
    QString loginData = QString::fromStdString(BufferToString(msg.loginData));
    bool success = xml.setContent(loginData);
    if (!success)
        TundraLogicModule::LogWarning("Received malformed xml logindata from user " + ToString<int>(user->userID));
    
    // Fill the user's logindata, both in raw format and as keyvalue pairs
    user->loginData = loginData;
    QDomElement rootElem = xml.firstChildElement();
    QDomElement keyvalueElem = rootElem.firstChildElement();
    while (!keyvalueElem.isNull())
    {
        //TundraLogicModule::LogInfo("Logindata contains keyvalue pair " + keyvalueElem.tagName().toStdString() + " = " + keyvalueElem.attribute("value").toStdString());
        user->SetProperty(keyvalueElem.tagName(), keyvalueElem.attribute("value"));
        keyvalueElem = keyvalueElem.nextSiblingElement();
    }
    
    user->properties["authenticated"] = "true";
    emit UserAboutToConnect(user->userID, user);
    if (user->properties["authenticated"] != "true")
    {
        TundraLogicModule::LogInfo("User with connection ID " + ToString<int>(user->userID) + " was denied access");
        MsgLoginReply reply;
        reply.success = 0;
        reply.userID = 0;
        user->connection->Send(reply);
        return;
    }
    
    TundraLogicModule::LogInfo("User with connection ID " + ToString<int>(user->userID) + " logged in");
    
    // Allow entityactions & EC sync from now on
    MsgLoginReply reply;
    reply.success = 1;
    reply.userID = user->userID;
    
    // Tell everyone of the client joining (also the user who joined)
    UserConnectionList users = GetAuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        (*iter)->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if ((*iter)->userID != user->userID)
        {
            MsgClientJoined joined;
            joined.userID = (*iter)->userID;
            user->connection->Send(joined);
        }
    }
    
    // Tell syncmanager of the new user
    owner_->GetSyncManager()->NewUserConnected(user);
    
    // Tell all server-side application code that a new user has successfully connected.
    // Ask them to fill the contents of a UserConnectedResponseData structure. This will
    // be sent to the client so that the scripts and applications on the client system can configure themselves.
    UserConnectedResponseData responseData;
    emit UserConnected(user->userID, user, &responseData);

    QByteArray responseByteData = responseData.responseData.toByteArray(-1);
    reply.loginReplyData.insert(reply.loginReplyData.end(), responseByteData.data(), responseByteData.data() + responseByteData.size());
    user->connection->Send(reply);
}

void Server::HandleUserDisconnected(UserConnection* user)
{
    // Tell everyone of the client leaving
    MsgClientLeft left;
    left.userID = user->userID;
    UserConnectionList users = GetAuthenticatedUsers();
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if ((*iter)->userID != user->userID)
            (*iter)->connection->Send(left);
    }
    
    emit UserDisconnected(user->userID, user);
}

template<typename T>
QScriptValue qScriptValueFromNull(QScriptEngine *engine, const T &v)
{
    return QScriptValue();
}

template<typename T>
void qScriptValueToNull(const QScriptValue &value, T &v)
{
}

void Server::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<UserConnection*>(engine);
    ///\todo Write proper serialization and deserialization.
    qScriptRegisterMetaType<UserConnectedResponseData*>(engine, qScriptValueFromNull<UserConnectedResponseData*>, qScriptValueToNull<UserConnectedResponseData*>);
}

}
