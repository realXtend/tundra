// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Server.h"
#include "TundraLogicModule.h"
#include "ModuleManager.h"
#include "SyncManager.h"
#include "KristalliProtocolModule.h"
#include "CoreStringUtils.h"
#include "TundraMessages.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"

#include "SceneAPI.h"
#include "ConfigAPI.h"

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
}

Server::~Server()
{
}

void Server::Update(f64 frametime)
{
}

bool Server::Start(unsigned short port)
{
    if (owner_->IsServer())
        return true; // Already started, don't need to do anything.

    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_SERVER, "protocol", "tcp");

    // Write values if config does not have them. First start or config was removed.
    if (!framework_->Config()->HasValue(configData))
        framework_->Config()->Set(configData);

    // Set default protocol
    kNet::SocketTransportLayer transportLayer = owner_->GetKristalliModule()->defaultTransport;

    // Read --protocol or config
    QString userSetProtocol;
    if (framework_->ProgramOptions().count("protocol") == 0)
    {
        userSetProtocol = framework_->Config()->Get(configData).toString().toLower();
    }
    else
    {
        try
        {
            userSetProtocol = QString::fromStdString(framework_->ProgramOptions()["protocol"].as<std::string>());
        }
        catch(...) {}
    }

    // Inspect protocol
    if (userSetProtocol != "udp" && userSetProtocol != "tcp")
        ::LogWarning("Server::Start: Server config has an invalid server protocol '" + userSetProtocol + "'. Use tcp or udp. Resetting to default protocol.");
    else
        transportLayer = userSetProtocol == "udp" ? kNet::SocketOverUDP : kNet::SocketOverTCP;
    
    // Start server
    if (!owner_->GetKristalliModule()->StartServer(port, transportLayer))
    {
        ::LogError("Failed to start server in port " + ToString<int>(port));
        return false;
    }

    // Store current port and protocol
    current_port_ = (int)port;
    current_protocol_ = transportLayer == kNet::SocketOverUDP ? "udp" : "tcp";

    // Create the default server scene
    /// \todo Should be not hard coded like this. Give some unique id (uuid perhaps) that could be returned to the client to make the corresponding named scene in client?
    ScenePtr scene = framework_->Scene()->CreateScene("TundraServer", true);
    framework_->Scene()->SetDefaultScene(scene);
    owner_->GetSyncManager()->RegisterToScene(scene);

    // Create an authoritative physics world
    Physics::PhysicsModule *physics = framework_->GetModule<Physics::PhysicsModule>();
    physics->CreatePhysicsWorldForScene(scene, false);
            
    emit ServerStarted();

    KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);

    connect(kristalli, SIGNAL(ClientDisconnectedEvent(UserConnection *)), this, SLOT(HandleUserDisconnected(UserConnection *)), Qt::UniqueConnection);

    return true;
}

void Server::Stop()
{
    if (owner_->IsServer())
    {
        ::LogInfo("Stopped Tundra server. Removing TundraServer scene.");

        owner_->GetKristalliModule()->StopServer();
        framework_->Scene()->RemoveScene("TundraServer");
        
        emit ServerStopped();

        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        disconnect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)));

        disconnect(kristalli, SIGNAL(ClientDisconnectedEvent(UserConnection *)), this, SLOT(HandleUserDisconnected(UserConnection *)));
    }
}

bool Server::IsRunning() const
{
    return owner_->IsServer();
}

bool Server::IsAboutToStart() const
{
    const boost::program_options::variables_map &programOptions = framework_->ProgramOptions();
    if (programOptions.count("server"))
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
    for(UserConnectionList::const_iterator iter = all.begin(); iter != all.end(); ++iter)
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
    for(UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        ret.push_back(QVariant((*iter)->userID));
    
    return ret;
}

UserConnection* Server::GetUserConnection(int connectionID) const
{
    UserConnectionList users = GetAuthenticatedUsers();
    for(UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
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

kNet::NetworkServer *Server::GetServer() const
{
    if (!owner_ || !owner_->GetKristalliModule())
        return 0;

    return owner_->GetKristalliModule()->GetServer();
}

void Server::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
{
    if (!source)
        return;

    if (!owner_->IsServer())
        return;

    UserConnection *user = GetUserConnection(source);
    if (!user)
    {
        ::LogWarning("Server: dropping message " + ToString(id) + " from unknown connection \"" + source->ToString() + "\"");
        return;
    }

    // If we are server, only allow the login message from an unauthenticated user
    if (id != cLoginMessage && user->properties["authenticated"] != "true")
    {
        UserConnection* user = GetUserConnection(source);
        if ((!user) || (user->properties["authenticated"] != "true"))
        {
            ::LogWarning("Server: dropping message " + ToString(id) + " from unauthenticated user");
            /// \todo something more severe, like disconnecting the user
            return;
        }
    }
    else if (id == cLoginMessage)
    {
        MsgLogin msg(data, numBytes);
        HandleLogin(source, msg);
    }

    emit MessageReceived(user, id, data, numBytes);
}

void Server::HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg)
{
    UserConnection* user = GetUserConnection(source);
    if (!user)
    {
        ::LogWarning("Login message from unknown user");
        return;
    }
    
    QDomDocument xml;
    QString loginData = QString::fromStdString(BufferToString(msg.loginData));
    bool success = xml.setContent(loginData);
    if (!success)
        ::LogWarning("Received malformed xml logindata from user " + ToString<int>(user->userID));
    
    // Fill the user's logindata, both in raw format and as keyvalue pairs
    user->loginData = loginData;
    QDomElement rootElem = xml.firstChildElement();
    QDomElement keyvalueElem = rootElem.firstChildElement();
    while(!keyvalueElem.isNull())
    {
        //::LogInfo("Logindata contains keyvalue pair " + keyvalueElem.tagName().toStdString() + " = " + keyvalueElem.attribute("value").toStdString());
        user->SetProperty(keyvalueElem.tagName(), keyvalueElem.attribute("value"));
        keyvalueElem = keyvalueElem.nextSiblingElement();
    }
    
    user->properties["authenticated"] = "true";
    emit UserAboutToConnect(user->userID, user);
    if (user->properties["authenticated"] != "true")
    {
        ::LogInfo("User with connection ID " + ToString<int>(user->userID) + " was denied access");
        MsgLoginReply reply;
        reply.success = 0;
        reply.userID = 0;
        user->connection->Send(reply);
        return;
    }
    
    ::LogInfo("User with connection ID " + ToString<int>(user->userID) + " logged in");
    
    // Allow entityactions & EC sync from now on
    MsgLoginReply reply;
    reply.success = 1;
    reply.userID = user->userID;
    
    // Tell everyone of the client joining (also the user who joined)
    UserConnectionList users = GetAuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    for(UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        (*iter)->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    for(UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
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
    for(UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
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
