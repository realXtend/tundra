// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Server.h"
#include "TundraLogicModule.h"
#include "SyncManager.h"
#include "KristalliProtocolModule.h"
#include "TundraMessages.h"
#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"
#include "UserConnectedResponseData.h"

#include "CoreStringUtils.h"
#include "SceneAPI.h"
#include "ConfigAPI.h"
#include "LoggingFunctions.h"
#include "QScriptEngineHelpers.h"

#include <QtScript>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(UserConnection*);
Q_DECLARE_METATYPE(UserConnectionPtr);
Q_DECLARE_METATYPE(UserConnectionList);
Q_DECLARE_METATYPE(TundraLogic::SyncManager*);
Q_DECLARE_METATYPE(SceneSyncState*);
Q_DECLARE_METATYPE(StateChangeRequest*);
Q_DECLARE_METATYPE(UserConnectedResponseData*);
Q_DECLARE_METATYPE(LoginPropertyMap);

using namespace kNet;

namespace TundraLogic
{

Server::Server(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    current_port_(-1)
{
}

Server::~Server()
{
}

void Server::Update(f64 frametime)
{
}

bool Server::Start(unsigned short port, QString protocol)
{
    if (owner_->IsServer())
    {
        LogDebug("[SERVER] Trying to start server but it's already running.");
        return true; // Already started, don't need to do anything.
    }

    // Protocol is usually defined as a --protocol command line parameter or in config file,
    // but if it's given as a param to this function use it instead.
    if (protocol.isEmpty() && framework_->HasCommandLineParameter("--protocol"))
    {
        QStringList cmdLineParams = framework_->CommandLineParameters("--protocol");
        if (cmdLineParams.size() > 0)
            protocol = cmdLineParams[0];
        else
            ::LogError("--protocol specified without a parameter! Using UDP protocol as default.");
    }

    kNet::SocketTransportLayer transportLayer = StringToSocketTransportLayer(protocol.trimmed().toStdString().c_str());
    if (transportLayer == kNet::InvalidTransportLayer)
    {
        ::LogError("Invalid server protocol '" + protocol + "' specified! Using UDP protocol as default.");
        transportLayer = kNet::SocketOverUDP; // By default operate over UDP.
    }

    // Start server
    if (!owner_->GetKristalliModule()->StartServer(port, transportLayer))
    {
        ::LogError("[SERVER] Failed to start server in port " + QString::number(port));
        return false;
    }

    // Store current port and protocol
    current_port_ = (int)port;
    current_protocol_ = (transportLayer == kNet::SocketOverUDP) ? "udp" : "tcp";

    // Create the default server scene
    /// \todo Should be not hard coded like this. Give some unique id (uuid perhaps) that could be returned to the client to make the corresponding named scene in client?
    ScenePtr scene = framework_->Scene()->CreateScene("TundraServer", true, true);
//    framework_->Scene()->SetDefaultScene(scene);
    owner_->GetSyncManager()->RegisterToScene(scene);

    emit ServerStarted();

    KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);

    connect(kristalli, SIGNAL(ClientDisconnectedEvent(UserConnection *)), this, SLOT(HandleUserDisconnected(UserConnection *)), Qt::UniqueConnection);

    return true;
}

void Server::Stop()
{
    if (owner_->IsServer())
    {
        ::LogInfo("[SERVER] Stopped Tundra server. Removing TundraServer scene.");

        owner_->GetKristalliModule()->StopServer();
        framework_->Scene()->RemoveScene("TundraServer");
        
        emit ServerStopped();

        KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
        disconnect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)), 
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)));

        disconnect(kristalli, SIGNAL(ClientDisconnectedEvent(UserConnection *)), this, SLOT(HandleUserDisconnected(UserConnection *)));
    }
}

bool Server::IsRunning() const
{
    return owner_->IsServer();
}

bool Server::IsAboutToStart() const
{
    return framework_->HasCommandLineParameter("--server");
}

int Server::Port() const
{
    return IsRunning() ? current_port_ : -1;
}

QString Server::Protocol() const
{
    return IsRunning() ? current_protocol_ : "";
}

int Server::GetPort() const
{
    LogWarning("Server::GetPort: This function signature is deprecated will be removed. Migrate to using Port or 'port' property instead.");
    return Port();
}

QString Server::GetProtocol() const
{
    LogWarning("Server::GetProtocol: This function signature is deprecated will be removed. Migrate to using Protocol or 'protocol' property instead.");
    return Protocol();
}

UserConnectionPtr Server::GetActionSender() const
{
    LogWarning("Server::GetActionSender: This function signature is deprecated will be removed. Migrate to using ActionSender instead.");
    return ActionSender();
}

UserConnectionList Server::AuthenticatedUsers() const
{
    UserConnectionList ret;
    foreach(const UserConnectionPtr &user, UserConnections())
        if (user->properties["authenticated"] == "true")
            ret.push_back(user);
    return ret;
}

QVariantList Server::GetConnectionIDs() const
{
    /// @todo Add warning print
//    LogWarning("Server::GetConnectionIDs: This function signature is deprecated will be removed. Migrate to using AuthenticatedUsers instead.");
    QVariantList ret;
    foreach(const UserConnectionPtr &user, AuthenticatedUsers())
        ret.push_back(QVariant((uint)user->userID)); /**< @todo The uint cast should not be necessary here, but without it when compiling TUNDRA_NO_BOOST build with VC9 we get error C2440: '<function-style-cast>' : cannot convert from 'u32' to 'QVariant' */
    return ret;
}

UserConnectionPtr Server::GetUserConnection(unsigned int connectionID) const
{
    foreach(const UserConnectionPtr &user, AuthenticatedUsers())
        if (user->userID == connectionID)
            return user;
    return UserConnectionPtr();
}

UserConnectionList& Server::UserConnections() const
{
    return owner_->GetKristalliModule()->GetUserConnections();
}

UserConnectionPtr Server::GetUserConnection(kNet::MessageConnection* source) const
{
    return owner_->GetKristalliModule()->GetUserConnection(source);
}

void Server::SetActionSender(const UserConnectionPtr &user)
{
    actionSender = user;
}

UserConnectionPtr Server::ActionSender() const
{
    return actionSender.lock();
}

kNet::NetworkServer *Server::GetServer() const
{
    if (!owner_ || !owner_->GetKristalliModule())
        return 0;

    return owner_->GetKristalliModule()->GetServer();
}

void Server::HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes)
{
    if (!source)
        return;

    if (!owner_->IsServer())
        return;

    UserConnectionPtr user = GetUserConnection(source);
    if (!user)
    {
        ::LogWarning(QString("Server: dropping message %1 from unknown connection \"%2\".").arg(messageId).arg(source->ToString().c_str()));
        return;
    }

    // If we are server, only allow the login message from an unauthenticated user
    if (messageId != MsgLogin::messageID && user->properties["authenticated"] != "true")
    {
        UserConnectionPtr user = GetUserConnection(source);
        if (!user || user->properties["authenticated"] != "true")
        {
            ::LogWarning("Server: dropping message " + QString::number(messageId) + " from unauthenticated user.");
            /// \todo something more severe, like disconnecting the user
            return;
        }
    }
    else if (messageId == MsgLogin::messageID)
    {
        MsgLogin msg(data, numBytes);
        HandleLogin(source, msg);
    }

    emit MessageReceived(user.get(), packetId, messageId, data, numBytes);
}

void Server::HandleLogin(kNet::MessageConnection* source, const MsgLogin& msg)
{
    UserConnectionPtr user = GetUserConnection(source);
    if (!user)
    {
        ::LogWarning("[SERVER] Login message from unknown user");
        return;
    }
    
    QDomDocument xml;
    QString loginData = QString::fromStdString(BufferToString(msg.loginData));
    bool success = xml.setContent(loginData);
    if (!success)
        ::LogWarning(QString("[SERVER] ID %1 client login data xml has malformed data").arg(user->userID));
    
    // Fill the user's logindata, both in raw format and as keyvalue pairs
    user->loginData = loginData;
    QDomElement rootElem = xml.firstChildElement();
    QDomElement keyvalueElem = rootElem.firstChildElement();
    while(!keyvalueElem.isNull())
    {
        //::LogInfo("Logindata contains keyvalue pair " + keyvalueElem.tagName() + " = " + keyvalueElem.attribute("value);
        user->SetProperty(keyvalueElem.tagName(), keyvalueElem.attribute("value"));
        keyvalueElem = keyvalueElem.nextSiblingElement();
    }

    QString connectedUsername = user->GetProperty("username");
    
    user->properties["authenticated"] = "true";
    emit UserAboutToConnect(user->userID, user.get());
    if (user->properties["authenticated"] != "true")
    {
        if (connectedUsername.isEmpty())
            ::LogInfo(QString("[SERVER] ID %1 client was denied access [%2] ").arg(user->userID).arg(user->connection->RemoteEndPoint().ToString().c_str()));
        else
            ::LogInfo(QString("[SERVER] ID %1 client '%2' was denied access [%3] ").arg(user->userID).arg(connectedUsername).arg(user->connection->RemoteEndPoint().ToString().c_str()));
        MsgLoginReply reply;
        reply.success = 0;
        reply.userID = 0;
        QByteArray responseByteData = user->properties["reason"].toAscii();
        reply.loginReplyData.insert(reply.loginReplyData.end(), responseByteData.data(), responseByteData.data() + responseByteData.size());
        user->connection->Send(reply);
        return;
    }
    
    if (connectedUsername.isEmpty())
        ::LogInfo(QString("[SERVER] ID %1 client connected - %2 ").arg(user->userID).arg(user->connection->RemoteEndPoint().ToString().c_str()));
    else
        ::LogInfo(QString("[SERVER] ID %1 client '%2' connected [%3] ").arg(user->userID).arg(connectedUsername).arg(user->connection->RemoteEndPoint().ToString().c_str()));
    
    // Allow entityactions & EC sync from now on
    MsgLoginReply reply;
    reply.success = 1;
    reply.userID = user->userID;
    
    // Tell everyone of the client joining (also the user who joined)
    UserConnectionList users = AuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    foreach(const UserConnectionPtr &u, users)
        u->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    foreach(const UserConnectionPtr &u, users)
        if (u->userID != user->userID)
        {
            MsgClientJoined joined;
            joined.userID = u->userID;
            user->connection->Send(joined);
        }
    
    // Tell syncmanager of the new user
    owner_->GetSyncManager()->NewUserConnected(user);
    
    // Tell all server-side application code that a new user has successfully connected.
    // Ask them to fill the contents of a UserConnectedResponseData structure. This will
    // be sent to the client so that the scripts and applications on the client system can configure themselves.
    UserConnectedResponseData responseData;
    emit UserConnected(user->userID, user.get(), &responseData);

    QByteArray responseByteData = responseData.responseData.toByteArray(-1);
    reply.loginReplyData.insert(reply.loginReplyData.end(), responseByteData.data(), responseByteData.data() + responseByteData.size());
    user->connection->Send(reply);
}

void Server::HandleUserDisconnected(UserConnection* user)
{
    // Tell everyone of the client leaving
    MsgClientLeft left;
    left.userID = user->userID;
    foreach(const UserConnectionPtr &u, AuthenticatedUsers())
        if (u->userID != user->userID)
            u->connection->Send(left);

    emit UserDisconnected(user->userID, user);

    QString username = user->GetProperty("username");
    if (username.isEmpty())
        ::LogInfo(QString("[SERVER] ID %1 client disconnected").arg(user->userID));
    else
        ::LogInfo(QString("[SERVER] ID %1 client '%2' disconnected").arg(user->userID).arg(username));
}

namespace
{

template<typename T>
QScriptValue qScriptValueFromNull(QScriptEngine *engine, const T &v)
{
    return QScriptValue();
}

template<typename T>
void qScriptValueToNull(const QScriptValue &value, T &v)
{
}

void fromScriptValueUserConnectionList(const QScriptValue &obj, UserConnectionList &ents)
{
    ents.clear();
    QScriptValueIterator it(obj);
    while(it.hasNext())
    {
        it.next();
        UserConnection *u = qobject_cast<UserConnection *>(it.value().toQObject());
        if (u)
            ents.push_back(u->shared_from_this());
    }
}

QScriptValue toScriptValueUserConnectionList(QScriptEngine *engine, const UserConnectionList &cons)
{
    QScriptValue scriptValue = engine->newArray();
    int i = 0;
    for(UserConnectionList::const_iterator iter = cons.begin(); iter != cons.end(); ++iter)
    {
        scriptValue.setProperty(i, engine->newQObject((*iter).get()));
        ++i;
    }
    return scriptValue;
}

QScriptValue qScriptValueFromLoginPropertyMap(QScriptEngine *engine, const LoginPropertyMap &map)
{
    // Expose the login properties as a JavaScript _associative_ array.
    QScriptValue v = engine->newObject();
    for(LoginPropertyMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
        v.setProperty((*iter).first, (*iter).second);
    return v;
}

void qScriptValueToLoginPropertyMap(const QScriptValue &value, LoginPropertyMap &map)
{
    map.clear();
    QScriptValueIterator it(value);
    while(it.hasNext())
    {
        it.next();
        map[it.name()] = it.value().toString();
    }
}

} // ~unnamed namespace

void Server::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<UserConnection*>(engine);
    qScriptRegisterQObjectMetaType<SyncManager*>(engine);
    qScriptRegisterQObjectMetaType<SceneSyncState*>(engine);
    qScriptRegisterQObjectMetaType<StateChangeRequest*>(engine);
    ///\todo Write proper serialization and deserialization.
    qScriptRegisterMetaType<UserConnectedResponseData*>(engine, qScriptValueFromNull<UserConnectedResponseData*>, qScriptValueToNull<UserConnectedResponseData*>);
    qScriptRegisterMetaType<UserConnectionPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    qScriptRegisterMetaType<UserConnectionList>(engine, toScriptValueUserConnectionList, fromScriptValueUserConnectionList);
    qScriptRegisterMetaType<LoginPropertyMap>(engine, qScriptValueFromLoginPropertyMap, qScriptValueToLoginPropertyMap);
}

}
