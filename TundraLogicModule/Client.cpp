// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Client.h"
#include "TundraLogicModule.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "CoreStringUtils.h"
#include "SyncManager.h"
#include "TundraMessages.h"
#include "TundraEvents.h"
#include "PhysicsModule.h"

#include "SceneAPI.h"
#include "SceneManager.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

#include <QDomElement>

#include <QMapIterator>
#include <QMutableMapIterator>

using namespace kNet;

namespace TundraLogic
{

Client::Client(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    loginstate_(NotConnected),
    reconnect_(false),
    client_id_(0),
    connectionsAvailable(false)
{
    tundraEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Tundra");
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");

    // Multiconnection specific variables
    loginstate_list_.clear();
    client_id_list_.clear();
    reconnect_list_.clear();
    properties_list_.clear();

    connect(owner_, SIGNAL(setClientActiveConnection(QString, unsigned short)), this, SLOT(setActiveConnection(QString, unsigned short)));
}

Client::~Client()
{
    while (!scenenames_.isEmpty())
        Logout(false, owner_->Grep(scenenames_.begin().value()));
}

void Client::Update(f64 frametime)
{
    // If we aren't a server, check pending login
    if (!owner_->IsServer())
        CheckLogin();
}

void Client::Login(const QUrl& loginUrl)
{
    // We support tundra, http and https scheme login urls
    QString urlScheme = loginUrl.scheme().toLower();
    if (urlScheme.isEmpty())
        return;
    if (urlScheme != "tundra" && 
        urlScheme != "http" && 
        urlScheme != "https")
        return;

    // Parse values from url
    QString username = loginUrl.queryItemValue("username").trimmed();
    QString password = loginUrl.queryItemValue("password");
    QString avatarurl = loginUrl.queryItemValue("avatarurl").trimmed();
    QString protocol = loginUrl.queryItemValue("protocol").trimmed().toLower();
    QString address = loginUrl.host();
    int port = loginUrl.port();

    // Validation: Username and address is the minimal set that with we can login with
    if (username.isEmpty() || address.isEmpty())
        return;
    if (username.count(" ") > 0)
        username = username.replace(" ", "-");
    if (port < 0)
        port = 2345;

    // Set custom login parameters and login
    if (!avatarurl.isEmpty())
        SetLoginProperty("avatarurl", avatarurl);

    Login(address, port, username, password, protocol);
}

void Client::Login(const QString& address, unsigned short port, const QString& username, const QString& password, const QString &protocol)
{
    // Check if we already have a connection to this specific IP:port and if so, then switch to it
    if (checkIfConnected(address, QString::number(port), protocol))
        return;

    SetLoginProperty("address", address);
    SetLoginProperty("port", QString::number(port));
    SetLoginProperty("username", username);
    SetLoginProperty("password", password);
    SetLoginProperty("protocol", protocol);

    kNet::SocketTransportLayer transportLayer = kNet::InvalidTransportLayer;
    if (protocol.toLower() == "tcp")
        transportLayer = kNet::SocketOverTCP;
    else if (protocol.toLower() == "udp")
        transportLayer = kNet::SocketOverUDP;
    Login(address, port, transportLayer);
}

void Client::Login(const QString& address, unsigned short port, kNet::SocketTransportLayer protocol)
{
    if (owner_->IsServer())
    {
        TundraLogicModule::LogError("Already running a server, cannot login to a world as a client");
        return;
    }

    reconnect_ = false;
    if (protocol == kNet::InvalidTransportLayer)
    {
        TundraLogicModule::LogInfo("Client::Login: No protocol specified, using the default value.");
        protocol = owner_->GetKristalliModule()->defaultTransport;
    }

    QString newConName = getUniqueSceneName();

    owner_->GetKristalliModule()->Connect(address.toStdString().c_str(), port, protocol);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
    saveProperties(newConName);
}

void Client::Logout(bool fail, unsigned short removedConnection_)
{
    QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> sourceIterator = owner_->GetKristalliModule()->GetConnectionArray();

    if (!sourceIterator.hasNext())
        return;

    emit changeTab(removedConnection_);
    // Scene to be removed is TundraClient_X | X = 0, 1, 2, 3, ..., n; n ¤ Z+
    // removedConnection_ indicates which scene we are about to disconnect.
    QString sceneToRemove = "TundraClient_";
    sceneToRemove.append(QString("%1").arg(removedConnection_));

    if (loginstate_list_[sceneToRemove] != NotConnected)
    {
        // This signal is catched in TundraLogicModule.cpp. This changes scene to soonToBeDisconnected scene.
        emit aboutToDisconnect(sceneToRemove);
        if (GetConnection(removedConnection_))
        {
            owner_->GetKristalliModule()->Disconnect(fail, removedConnection_);
            TundraLogicModule::LogInfo(sceneToRemove.toStdString() + " disconnected!");
        }
        
        loginstate_ = NotConnected;
        client_id_ = 0;
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_DISCONNECTED, 0);
        framework_->Scene()->RemoveScene(sceneToRemove);

        // We remove TundraClient_X from the scenenames_ map and when next new connection happens
        // we create TundraClient_X again to fill the list.
        // scenenames_ has unsigned int as key so removedConnection_ clears right item from list.
        loginstate_list_.remove(sceneToRemove);
        reconnect_list_.remove(sceneToRemove);
        client_id_list_.remove(sceneToRemove);
        properties_list_.remove(sceneToRemove);
        scenenames_.remove(removedConnection_);

        // Check if we have connections up and running and switch to it.
        if (!scenenames_.isEmpty())
        {
            owner_->changeScene(scenenames_.constBegin().value());
            emit Disconnected(removedConnection_);

            // This is for LoginWebScreen.js tabChange method.
            QList<int> key = scenenames_.keys();
            emit changeTab(key[0]);
        }
        else
        {
            connectionsAvailable = false;
            emit Disconnected(removedConnection_);
        }
    }
    
    if (fail)
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_LOGIN_FAILED, 0);
    else // An user deliberately disconnected from the world, and not due to a connection error.
    {
        // Clear all the login properties we used for this session, so that the next login session will start from an
        // empty set of login properties (just-in-case).
        properties.clear();
    }
}

bool Client::IsConnected() const
{
    return loginstate_ == LoggedIn;
}

bool Client::checkIfConnected(QString address, QString port, QString protocol)
{
    unsigned short counter = 0;
    QMapIterator<QString, std::map<QString, QString> > propertiesIterator(properties_list_);

    while (propertiesIterator.hasNext())
    {
        propertiesIterator.next();
        std::map<QString, QString> temp = propertiesIterator.value();
        QString tempAddress = temp["address"];
        QString tempPort = temp["port"];
        QString tempProtocol = temp["protocol"];

        if (address == tempAddress && port == tempPort && protocol == tempProtocol)
        {
            // If this is true, we got loginscreen connect-button smasher user. :)
            if (scenenames_.size() < properties_list_.size())
                return false;
            QList<int> key = scenenames_.keys();
            unsigned short keyInt = key[counter];
            TundraLogicModule::LogInfo("Already connected to " + tempAddress.toStdString() + ":" + tempPort.toStdString() + ". Emitting " + ToString(keyInt));
            emit changeTab(keyInt);
            return true;
        }
        counter++;
    }
    return false;
}

void Client::SetLoginProperty(QString key, QString value)
{
    key = key.trimmed();
    value = value.trimmed();
    if (value.isEmpty())
        properties.erase(key);
    properties[key] = value;
}

QString Client::GetLoginProperty(QString key)
{
    // Multiconnection addition. Check what connection is active and set property.
    properties = properties_list_["TundraClient_" + QString::number(activeConnection)];
    key = key.trimmed();
    if (properties.count(key) > 0)
        return properties[key];
    else
        return "";
}

QString Client::LoginPropertiesAsXml() const
{
    QDomDocument xml;
    QDomElement rootElem = xml.createElement("login");
    for(std::map<QString, QString>::const_iterator iter = properties.begin(); iter != properties.end(); ++iter)
    {
        QDomElement elem = xml.createElement(iter->first.toStdString().c_str());
        elem.setAttribute("value", iter->second.toStdString().c_str());
        rootElem.appendChild(elem);
    }
    xml.appendChild(rootElem);
    return xml.toString();
}

void Client::CheckLogin()
{
    // Using iterators to process through all properties for established connections
    QMutableMapIterator<QString, ClientLoginState> loginstateIterator(loginstate_list_);
    QMapIterator<QString, std::map<QString, QString> > propertiesIterator(properties_list_);
    QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> connectionIterator = owner_->GetKristalliModule()->GetConnectionArray();

    // Checklogin only happens if atleast one connection is made in KristalliProtocolModule and set to ConnectionOK state.
    while (connectionIterator.hasNext() && loginstateIterator.hasNext())
    {
        connectionIterator.next();
        propertiesIterator.next();
        loginstateIterator.next();

        // If we have multiple connections and one of them gets disconnected, our serverconnection map has "missing" key
        // while client has properties for it, if it is making new connection. When this happens we compare if serverConnection
        // key is higher of value than loginstateIterator key after we grep the number out of it. If so, we proceed to next item
        // in loginstate and properties iterator.
        unsigned short temp;

        while (true)
        {
            temp = owner_->Grep(loginstateIterator.key());

            if (temp < connectionIterator.key())
            {
                propertiesIterator.next();
                loginstateIterator.next();
            }
            else
                break;
        }

        switch (loginstateIterator.value())
        {
        case ConnectionPending:
            if ((connectionIterator.value().ptr()) && (connectionIterator.value().ptr()->GetConnectionState() == kNet::ConnectionOK))
            {
                Ptr(kNet::MessageConnection) messageSender = connectionIterator.value();
                loginstateIterator.value() = ConnectionEstablished;
                MsgLogin msg;
                emit AboutToConnect(); // This signal is used as a 'function call'. Any interested party can fill in
                // new content to the login properties of the client object, which will then be sent out on the line below.
                properties = propertiesIterator.value();
                msg.loginData = StringToBuffer(LoginPropertiesAsXml().toStdString());
                messageSender.ptr()->Send(msg);
            }
            break;
        case LoggedIn:
            // If we have logged in, but connection dropped, prepare to resend login
            if ((!connectionIterator.value().ptr()) || (connectionIterator.value().ptr()->GetConnectionState() != kNet::ConnectionOK))
                loginstateIterator.value() = ConnectionPending;
            break;

        }
    }
}

kNet::MessageConnection* Client::GetConnection(unsigned short con)
{
    return owner_->GetKristalliModule()->GetMessageConnection(con);
}

void Client::HandleKristalliEvent(event_id_t event_id, IEventData* data)
{
    if (event_id == KristalliProtocol::Events::NETMESSAGE_IN)
    {
        if (!owner_->IsServer())
        {
            KristalliProtocol::Events::KristalliNetMessageIn* eventData = checked_static_cast<KristalliProtocol::Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
        }
    }
    if (event_id == KristalliProtocol::Events::CONNECTION_FAILED)
    {
        KristalliProtocol::Events::KristalliConnectionFailed* eventData = checked_static_cast<KristalliProtocol::Events::KristalliConnectionFailed*>(data);
        Logout(true, eventData->removedConnection_);
    }
}

void Client::HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes)
{
    QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> sourceIterator = owner_->GetKristalliModule()->GetConnectionArray();

    // check if any of the client's messageConnections send the message
    while (sourceIterator.hasNext())
    {
        sourceIterator.next();

        if (source == sourceIterator.value().ptr())
            break;
        else if (source != sourceIterator.value().ptr() && sourceIterator.hasNext())
            continue;
        else
        {
            TundraLogicModule::LogWarning("Client: dropping message " + ToString(id) + " from unknown source");
            return;
        }

    }
    
    switch (id)
    {
    case cLoginReplyMessage:
        {
            MsgLoginReply msg(data, numBytes);
            HandleLoginReply(source, msg);
        }
        break;
    case cClientJoinedMessage:
        {
            MsgClientJoined msg(data, numBytes);
            HandleClientJoined(source, msg);
        }
        break;
    case cClientLeftMessage:
        {
            MsgClientLeft msg(data, numBytes);
            HandleClientLeft(source, msg);
        }
        break;
    }
}

void Client::HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    //client_id_ = msg.userID;   // This is here for now. Needed by simpleavatar.js ClientInitialization.

    if (msg.success)
    {
        // conNumber is used if we are reconnecting.
        unsigned short conNumber = 0;

        // Iterators for checking the source of the message and handling message correcly using right properties.
        QMutableMapIterator<QString, ClientLoginState> loginstateIterator(loginstate_list_);
        QMutableMapIterator<QString, u8> client_idIterator(client_id_list_);
        QMutableMapIterator<QString, bool> reconnectIterator(reconnect_list_);
        QMapIterator<unsigned short, Ptr(kNet::MessageConnection)> sourceIterator = owner_->GetKristalliModule()->GetConnectionArray();

        // This while loop locates which messageconnection send the message. When we know it we also know if it is a reconnect or not.
        while (sourceIterator.hasNext())
        {
            loginstateIterator.next();
            reconnectIterator.next();
            client_idIterator.next();
            sourceIterator.next();

            if (sourceIterator.value().ptr() == source)
            {
                break;
            }
            conNumber++;
        }

        loginstateIterator.value() = LoggedIn;
        client_idIterator.value() = msg.userID;
        connectionsAvailable = true;
        TundraLogicModule::LogInfo("Logged in successfully");
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnectIterator.value())
        {
            TundraLogicModule::LogInfo("Brand new connection (vs reconnect)");
            QString sceneName = getUniqueSceneName(true);
            Scene::ScenePtr scene = framework_->Scene()->CreateScene(sceneName, true);
            // Create physics world in client (non-authoritative) mode
            Physics::PhysicsModule *physics = framework_->GetModule<Physics::PhysicsModule>();
            physics->CreatePhysicsWorldForScene(scene, true);
            
            framework_->Scene()->SetDefaultScene(scene);
            
            Events::TundraConnectedEventData event_data;
            event_data.user_id_ = msg.userID;
            framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_CONNECTED, &event_data);
            
            emit Connected(conNumber);
        }
        else
        {
            // If we are reconnecting, empty the scene, as the server will send everything again anyway
            // Note: when we move to unordered communication, we must guarantee that the server does not send
            // any scene data before the login reply
            QString sceneToGet = "TundraClient_";
            sceneToGet.append(QString("%1").arg(conNumber));

            TundraLogicModule::LogInfo("Reconnecting " + sceneToGet.toStdString());
            Scene::ScenePtr scene = framework_->Scene()->GetScene(sceneToGet);
            if (scene)
                scene->RemoveAllEntities(true, AttributeChange::LocalOnly);
        }
        reconnectIterator.value() = true;
    }
    else
    {
        Logout(true);
    }
}

void Client::HandleClientJoined(MessageConnection* source, const MsgClientJoined& msg)
{
}

void Client::HandleClientLeft(MessageConnection* source, const MsgClientLeft& msg)
{
}

// For multiconnection: This is used to scan through scenenames and get unique scenename
// for established connection. This same name is transfered to syncManager map and to Ogre
// for new scenemanager. This way naali-scene, Ogre scenemanager and syncManager is tied with
// one and the same name.
QString Client::getUniqueSceneName(bool save)
{
    QString sceneName = "TundraClient_";
    bool flag = true;

    for (unsigned short x = 0; flag ;x++)
        if (!scenenames_.contains(x))
        {
            sceneName.append(QString("%1").arg(x));
            if (save)
                scenenames_.insert(x, sceneName);
            flag = false;
        }
    return sceneName;
}

void Client::saveProperties(const QString &name)
{
    // Container for all the connections loginstates
    loginstate_list_.insert(name, loginstate_);
    loginstate_ = NotConnected;
    // Container for all the connections reconnect bool value
    reconnect_list_.insert(name, reconnect_);
    reconnect_ = false;
    // Container for all the connections clientID values
    client_id_list_.insert(name, client_id_);
    client_id_ = 0;
    // Container for all the connections properties
    properties_list_.insert(name, properties);
    properties.clear();
}

// syncManager is created in TundraLogicModule but javascript can not connect signals from there.
// So we emit signal when TundraLogicModule creates syncManager which invokes this slot which creates
// connectable signal for javascripting. Ez mode! :]
void Client::emitCreateOgreSignal(const QString &name)
{
    emit createOgre(name);
}

void Client::emitDeleteOgreSignal(const QString &name)
{
    emit deleteOgre(name);
}

void Client::emitSetOgreSignal(const QString &name)
{
    emit setOgre(name);
}

void Client::emitChangeSceneSignal(const QString &name)
{
    QStringList list = name.split("_");
    QString number = list[1];
    unsigned short key = number.toInt();
    if (scenenames_.contains(key))
        emit changeScene(name);
}

void Client::setActiveConnection(const QString& name, unsigned short con)
{
    client_id_ = client_id_list_[name];
    activeConnection = con;
}

unsigned short Client::getActiveConnection() const
{
    return activeConnection;
}

bool Client::hasConnections()
{
    return connectionsAvailable;
}

}

