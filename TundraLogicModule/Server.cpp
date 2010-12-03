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
#include "TundraEvents.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

#include <QtScript>

Q_DECLARE_METATYPE(UserConnection*);

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
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject,
                                       qScriptValueToQObject, prototype);
}


using namespace kNet;

namespace TundraLogic
{

Server::Server(TundraLogicModule* owner, Foundation::Framework* fw) :
    owner_(owner),
    framework_(fw)
{
    tundraEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Tundra");
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");
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
        if (!owner_->GetKristalliModule()->StartServer(port, SocketOverTCP))
        {
            TundraLogicModule::LogError("Failed to start server in port " + ToString<int>(port));
            return false;
        }
        Scene::ScenePtr scene = framework_->CreateScene("TundraServer", true);
        framework_->SetDefaultWorldScene(scene);
        owner_->GetSyncManager()->RegisterToScene(scene);
        
        // We are server, so create physics world for the scene
        Physics::PhysicsModule *physics = framework_->GetModule<Physics::PhysicsModule>();
        if (physics)
        {
            Physics::PhysicsWorld* world = physics->CreatePhysicsWorldForScene(scene);
            world->SetGravity(Vector3df(0.0f,0.0f,-9.81f));
        }
        
        //! \todo Hack - find better way and remove! Allow environment also on server by sending a fake connect event
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
        framework_->RemoveScene("TundraServer");
        
        emit ServerStopped();
    }
}

bool Server::IsRunning() const
{
    return owner_->IsServer();
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
    // For now, automatically accept the connection if it's from a known user
    UserConnection* user = GetUserConnection(source);
    if (!user)
    {
        TundraLogicModule::LogWarning("Login message from unknown user");
        return;
    }
    
    //! \todo authentication check here as necessary
    
    user->userName = QString::fromStdString(BufferToString(msg.userName));
    user->properties["password"] = QString::fromStdString(BufferToString(msg.password));
    user->properties["authenticated"] = "true";
    
    TundraLogicModule::LogInfo("User " + user->userName.toStdString() + " logging in, connection ID " + ToString<int>(user->userID));
    
    MsgLoginReply reply;
    reply.success = 1;
    reply.userID = user->userID;
    user->connection->Send(reply);
    
    // Tell everyone of the client joining (also the user who joined)
    UserConnectionList users = GetAuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    joined.userName = msg.userName;
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        (*iter)->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if ((*iter)->userID != user->userID)
        {
            MsgClientJoined joined;
            joined.userID = (*iter)->userID;
            joined.userName = StringToBuffer((*iter)->userName.toStdString());
            user->connection->Send(joined);
        }
    }
    
    // Tell syncmanager of the new user
    owner_->GetSyncManager()->NewUserConnected(user);
    
    emit UserConnected(user->userID, user);
}

void Server::HandleUserDisconnected(UserConnection* user)
{
    // Tell everyone of the client leaving
    MsgClientLeft left;
    left.userID = user->userID;
    left.userName = StringToBuffer(user->userName.toStdString());
    UserConnectionList users = GetAuthenticatedUsers();
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if ((*iter)->userID != user->userID)
            (*iter)->connection->Send(left);
    }
    
    emit UserDisconnected(user->userID, user);
}

void Server::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<UserConnection*>(engine);
}

}
