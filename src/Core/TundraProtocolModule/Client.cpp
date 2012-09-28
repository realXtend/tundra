// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Client.h"
#include "TundraLogicModule.h"
#include "KristalliProtocolModule.h"
#include "SyncManager.h"
#include "TundraMessages.h"
#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"
#include "UserConnectedResponseData.h"

#include "LoggingFunctions.h"
#include "CoreStringUtils.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "AssetAPI.h"
#include "Application.h"

#include <kNet.h>

#include "MemoryLeakCheck.h"

using namespace kNet;

namespace TundraLogic
{

Client::Client(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    loginstate_(NotConnected),
    reconnect_(false),
    client_id_(0),
    discScene("")
{
}

Client::~Client()
{
}

void Client::Update(f64 /*frametime*/)
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
    if (urlScheme != "tundra" && urlScheme != "http" && urlScheme != "https")
        return;

    // If connected, just do nothing for now.
    if (IsConnected(loginUrl.host(),loginUrl.port(),loginUrl.queryItemValue("protocol")))
        return;

    // Set properties that the "lower" overload wont be adding:
    // Iterate all query items and parse them to go into the login properties.
    // This will leave percent encoding to the parameters! We remove it by hand from username below!
    QList<QPair<QString, QString> > queryItems = loginUrl.queryItems();
    for (int i=0; i<queryItems.size(); i++)
    {
        QPair<QString, QString> queryItem = queryItems.at(i);
        // Skip the ones that are handled by below logic
        if (queryItem.first == "username" || queryItem.first == "password" || queryItem.first == "protocol")
            continue;
        SetLoginProperty(queryItem.first, queryItem.second);
    }

    // Parse values from url
    QString username = loginUrl.queryItemValue("username");
    QString password = loginUrl.queryItemValue("password");
    QString protocol = loginUrl.queryItemValue("protocol");
    QString address = loginUrl.host();
    int port = loginUrl.port();

    // If the username is more exotic or has spaces, prefer
    // decoding the percent encoding before it is sent to the server.
    QByteArray utfUsername = loginUrl.queryItemValue("username").toUtf8();
    if (utfUsername.contains('%'))
    {
        // Use QUrl to decode percent encoding instead of QByteArray.
        username = QUrl::fromEncoded(utfUsername).toString();
    }

    // Validation: Username and address is the minimal set that with we can login with
    if (username.isEmpty() || address.isEmpty())
    {
        ::LogError("Client::Login: Cannot log to server, no username defined in login url: " + loginUrl.toString());
        return;
    }
    if (port < 0)
        port = 2345;

    Login(address, port, username, password, protocol);
}

void Client::Login(const QString& address, unsigned short port, const QString& username, const QString& password, const QString &protocol)
{
    if (IsConnected(address, port, protocol))
        return;

    // Set properties that the "lower" overload wont be adding.
    SetLoginProperty("username", username);
    SetLoginProperty("password", password);

    QString p = protocol.trimmed().toLower();
    kNet::SocketTransportLayer transportLayer = kNet::SocketOverUDP;
    if (p.compare("tcp", Qt::CaseInsensitive) == 0)
        transportLayer = kNet::SocketOverTCP;
    else if (p.compare("udp", Qt::CaseInsensitive) == 0)
        transportLayer = kNet::SocketOverUDP;
    else if (!p.trimmed().isEmpty())
    {
        ::LogError("Client::Login: Cannot log to server using unrecognized protocol: " + p);
        return;
    }
    Login(address, port, transportLayer);
}

void Client::Login(const QString& address, unsigned short port, kNet::SocketTransportLayer protocol)
{
    if (owner_->IsServer())
    {
        ::LogError("Already running a server, cannot login to a world as a client");
        return;
    }

    reconnect_ = false;
    
    if (protocol == kNet::InvalidTransportLayer)
    {
        ::LogInfo("Client::Login: No protocol specified, using the default value.");
        protocol = owner_->GetKristalliModule()->defaultTransport;
    }
    QString p = "";
    if (protocol == kNet::SocketOverTCP)
        p = "tcp";
    else if (protocol == kNet::SocketOverUDP)
        p = "udp";

    // Set all login properties we have knowledge of.
    // Others may have been added before calling this function.
    SetLoginProperty("protocol", p);
    SetLoginProperty("address", address);
    SetLoginProperty("port", QString::number(port));
    SetLoginProperty("client-version", Application::Version());
    SetLoginProperty("client-name", Application::ApplicationName());
    SetLoginProperty("client-organization", Application::OrganizationName());

    KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)),
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);
    connect(kristalli, SIGNAL(ConnectionAttemptFailed(QString&)), this, SLOT(OnConnectionAttemptFailed(QString&)), Qt::UniqueConnection);

    owner_->GetKristalliModule()->Connect(address.toStdString().c_str(), port, protocol);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
    // Save clientId, reconnect, loginstate etc
    saveProperties();
}

void Client::Logout(const QString &name)
{
    // Handle console disconnect with parameter but allow user to disconnect main camera scene from JS using client.disconnect()
        if (name != "")
            discScene = name;
        else
            discScene = framework_->Scene()->MainCameraScene()->Name();
        QTimer::singleShot(1, this, SLOT(DelayedLogout()));
}

void Client::DelayedLogout()
{
    DoLogout(false);
}

void Client::DoLogout(bool fail)
{

    QStringList keys = loginstate_list_.keys();
    if (!keys.contains(discScene))
    {
        discScene = "";
        printSceneNames();
        return;
    }

    if (loginstate_list_[discScene]!= NotConnected)
    {
        if (GetConnection(discScene))
        {
            owner_->GetKristalliModule()->Disconnect(discScene);
            ::LogInfo("Disconnected");
        }
        
        removeProperties(discScene);
        
        emit Disconnected(discScene);
    }
    
    if (fail)
    {
        QString failreason = LoginProperty("LoginFailed");
        emit LoginFailed(failreason);
    }
    else // An user deliberately disconnected from the world, and not due to a connection error.
    {
        // Clear all the login properties we used for this session, so that the next login session will start from an
        // empty set of login properties (just-in-case).
        if (client_id_list_.contains(discScene))
            removeProperties(discScene);
    }

    if (loginstate_list_.isEmpty())
    {
        KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
        disconnect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)),
                   this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)));

        disconnect(kristalli, SIGNAL(ConnectionAttemptFailed(QString&)), this, SLOT(OnConnectionAttemptFailed(QString&)));
    }
    framework_->Scene()->RemoveScene(discScene);
    ::LogInfo("Client logged out.");
}

bool Client::IsConnected() const
{
    return loginstate_ == LoggedIn;
}

bool Client::IsConnected(const QString& address, unsigned short port, const QString &protocol)
{
    QMap< QString, std::map<QString, QString> >::const_iterator iter = properties_list_.begin();
    QString tempProtocol = protocol;
    std::map<QString, QString> tempMap;
    if (protocol == "")
        tempProtocol = "udp";
    while (iter != properties_list_.end())
    {
        tempMap = iter.value();
        if (tempMap["address"] == address && tempMap["port"] == QString::number(port) && tempMap["protocol"] == tempProtocol)
        {
            setActiveScenename(iter.key());
            emit switchScene(iter.key());
            return true;
        }
        ++iter;
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

QString Client::LoginProperty(QString key) const
{
    LoginPropertyMap tempMap;

    if (properties_list_.contains(activescenename_))
        tempMap = properties_list_[activescenename_];
    else
        tempMap = properties_list_["NEW"];

    key = key.trimmed();
    LoginPropertyMap::const_iterator i = tempMap.find(key);

    if (i != tempMap.end())
        return i->second;
    else
        return "";
}

QString Client::LoginPropertiesAsXml() const
{
    QDomDocument xml;
    QDomElement rootElem = xml.createElement("login");
    for(LoginPropertyMap::const_iterator iter = properties.begin(); iter != properties.end(); ++iter)
    {
        QDomElement elem = xml.createElement(iter->first);
        elem.setAttribute("value", iter->second);
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
    QMapIterator<QString, Ptr(kNet::MessageConnection)> connectionIterator = owner_->GetKristalliModule()->GetConnectionArray();

    // Checklogin only happens if atleast one connection is made in KristalliProtocolModule and set to ConnectionOK state.
    while (connectionIterator.hasNext() && loginstateIterator.hasNext())
    {
        connectionIterator.next();
        propertiesIterator.next();
        loginstateIterator.next();
        //::LogInfo("Processing connection: " + connectionIterator.key() + " and loginstate: " + loginstateIterator.key());

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

kNet::MessageConnection* Client::GetConnection(const QString &name)
{
    return owner_->GetKristalliModule()->GetMessageConnection(name);
}

void Client::OnConnectionAttemptFailed(QString &key)
{
    QMap< QString, std::map<QString, QString> >::const_iterator iter = properties_list_.find(key);
    if (iter == properties_list_.end())
        return;
    properties = iter.value();

    // Provide a reason why the connection failed.
    QString address = LoginProperty("address");
    QString port = LoginProperty("port");
    QString protocol = LoginProperty("protocol");

    QString failReason = "Could not connect to host";
    if (!address.isEmpty())
    {
        failReason.append(" " + address);
        if (!port.isEmpty())
            failReason.append(":" + port);
        if (!protocol.isEmpty())
            failReason.append(" with " + protocol.toUpper());
    }

    SetLoginProperty("LoginFailed", failReason);
    discScene = key;
    DoLogout(true);
}

void Client::HandleKristalliMessage(MessageConnection* source, packet_id_t packetId, message_id_t messageId, const char* data, size_t numBytes)
{
    QMapIterator<QString, Ptr(kNet::MessageConnection)> sourceIterator = owner_->GetKristalliModule()->GetConnectionArray();

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
            ::LogWarning("Client: dropping message " + QString::number(messageId) + " from unknown source");
            return;
        }
    }

    switch(messageId)
    {
    case MsgLoginReply::messageID:
    {
        MsgLoginReply msg(data, numBytes);
        HandleLoginReply(source, msg);
    }
        break;
    case MsgClientJoined::messageID:
    {
        MsgClientJoined msg(data, numBytes);
        HandleClientJoined(source, msg);
    }
        break;
    case MsgClientLeft::messageID:
    {
        MsgClientLeft msg(data, numBytes);
        HandleClientLeft(source, msg);
    }
        break;
    }
    emit NetworkMessageReceived(packetId, messageId, data, numBytes);
}

void Client::HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    if (msg.success)
    {
        loginstate_ = LoggedIn;
        client_id_ = msg.userID;

        // Build scenename which is used as identifier for syncmanager/ogre/ScenePtr and scene specific data. Example sceneName: 127.0.0.1:2345:udp
        sceneName = QString::fromAscii(source->GetSocket()->DestinationAddress()) + "-" +QString::number(source->GetSocket()->DestinationPort());
        source->GetSocket()->TransportLayer() == kNet::SocketOverUDP ? sceneName.append("-udp") : sceneName.append("-tcp");

        activescenename_ = sceneName;
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnect_list_[sceneName])
        {
            // This sets identifier in KristalliProtocolModule for this particular connection
            // This is kinda ugly way to do this because this requires us to signal data back the
            // messaging chain in order to keep track of connection lists.
            owner_->GetKristalliModule()->SetIdentifier(sceneName);

            // Create a non-authoritative scene for the client
            ScenePtr scene = framework_->Scene()->CreateScene(sceneName, true, false);
            
            UserConnectedResponseData responseData;
            if (msg.loginReplyData.size() > 0)
                responseData.responseData.setContent(QByteArray((const char *)&msg.loginReplyData[0], (int)msg.loginReplyData.size()));

            emit Connected(&responseData);
        }
        else
        {
            // If we are reconnecting, empty the scene, as the server will send everything again anyway
            // Note: when we move to unordered communication, we must guarantee that the server does not send
            // any scene data before the login reply

            ScenePtr scene = framework_->Scene()->GetScene(sceneName);
            if (scene)
                scene->RemoveAllEntities(true, AttributeChange::LocalOnly);
        }
        reconnect_ = true;
        saveProperties(sceneName);
    }
    else
    {
        QString response(QByteArray((const char *)&msg.loginReplyData[0], (int)msg.loginReplyData.size()));
        if (!response.isEmpty())
            SetLoginProperty("LoginFailed", response);
        DoLogout(true);
    }
}

void Client::HandleClientJoined(MessageConnection* /*source*/, const MsgClientJoined& /*msg*/)
{
}

void Client::HandleClientLeft(MessageConnection* /*source*/, const MsgClientLeft& /*msg*/)
{
}

void Client::saveProperties(const QString name)
{
    // Login happened and replace NEW-marked properties with scenename.
    if (name != "NEW" && loginstate_list_.contains(name))
    {
        ::LogInfo("Reconnection saving properties " + name + "\n");
        // Container for all the connections loginstates
        loginstate_list_.insert(name, loginstate_);
        // Container for all the connections reconnect bool value
        reconnect_list_.insert(name, reconnect_);
        // Container for all the connections clientID values
        client_id_list_.insert(name, client_id_);
        // Container for all the connections properties
        //properties_list_.insert(name, properties_list_[name]);
        // Container for all the connections loginstates
    }
    else if (name != "NEW" && !loginstate_list_.contains(name))
    {
        ::LogInfo("NEW connection ready. Switching identifier to " + name + ".\n");
        // Container for all the connections loginstates
        loginstate_list_.insert(name, loginstate_);
        // Container for all the connections reconnect bool value
        reconnect_list_.insert(name, reconnect_);
        // Container for all the connections clientID values
        client_id_list_.insert(name, client_id_);
        // Container for all the connections properties
        properties_list_.insert(name, properties_list_["NEW"]);
        // Container for all the connections loginstates
        loginstate_list_.remove("NEW");
        loginstate_ = NotConnected;
        // Container for all the connections reconnect bool value
        reconnect_list_.remove("NEW");
        reconnect_ = false;
        // Container for all the connections clientID values
        client_id_list_.remove("NEW");
        // Container for all the connections properties
        properties_list_.remove("NEW");
        properties.clear();
    }
    else
    {
        // Initial connection attempt.

        ::LogInfo("New connection saving properties NEW.\n");
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
}

void Client::printSceneNames()
{
    QMap< QString, std::map<QString, QString> >::const_iterator iter = properties_list_.begin();
    if (iter == properties_list_.end())
        return;

    std::map<QString, QString> tempMap = iter.value();

    QStringList keys = properties_list_.keys();
    foreach (QString key, keys)
    {
        ::LogInfo("> " + key);//" - " + tempMap["address"] + ":" + tempMap["port"] + "/" + tempMap["protocol"] + "\n");
        ++iter;
        if (iter != properties_list_.end())
            tempMap = iter.value();
    }
}

QStringList Client::getSceneNames()
{
    // Used in javascript
    return loginstate_list_.keys();
}

void Client::removeProperties(const QString &name)
{
    loginstate_list_.remove(discScene);
    client_id_list_.remove(discScene);
    reconnect_list_.remove(discScene);
    properties_list_.remove(discScene);
}

void Client::emitSceneSwitch(const QString name)
{
    if (!loginstate_list_.contains(name))
        printSceneNames();
    else
    {
        ::LogInfo("Sceneswitch: " + name);
        activescenename_ = name;
        emit switchScene(name);
    }
}

}

