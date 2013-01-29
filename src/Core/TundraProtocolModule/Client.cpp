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
#include "MsgCameraOrientationRequest.h"
#include "UserConnectedResponseData.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"

#include "LoggingFunctions.h"
#include "CoreStringUtils.h"
#include "SceneAPI.h"
#include "Scene/Scene.h"
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
    sendCameraUpdates_(0),
    client_id_(0)
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

    // Make sure to logout to empty the previous properties map.
    if (IsConnected())
        DoLogout();

    // Set properties that the "lower" overload wont be adding:
    // Iterate all query items and parse them to go into the login properties.
    // This will leave percent encoding to the parameters! We remove it by hand from username below!
    QList<QPair<QString, QString> > queryItems = loginUrl.queryItems();
    for (int i=0; i<queryItems.size(); i++)
    {
        // Skip the ones that are handled by below logic
        QPair<QString, QString> queryItem = queryItems.at(i);
        if (queryItem.first == "username" || queryItem.first == "password" || queryItem.first == "protocol")
            continue;
        QByteArray utfQueryValue = queryItem.second.toUtf8();
        if (utfQueryValue.contains('%'))
        {
            // Use QUrl to decode percent encoding instead of QByteArray.
            queryItem.second = QUrl::fromEncoded(utfQueryValue).toString();
        }
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
    if (IsConnected())
        DoLogout();

    // Set properties that the "lower" overload wont be adding.
    SetLoginProperty("username", username);
    SetLoginProperty("password", password);

    std::string p = protocol.trimmed().toLower().toStdString();
    kNet::SocketTransportLayer transportLayer = StringToSocketTransportLayer(p.c_str());
    if (transportLayer == InvalidTransportLayer && !p.empty())
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
    // Set all login properties we have knowledge of. 
    // Others may have been added before calling this function.
    SetLoginProperty("protocol", QString(SocketTransportLayerToString(protocol).c_str()).toLower());
    SetLoginProperty("address", address);
    SetLoginProperty("port", QString::number(port));
    SetLoginProperty("client-version", Application::Version());
    SetLoginProperty("client-name", Application::ApplicationName());
    SetLoginProperty("client-organization", Application::OrganizationName());

    KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)), 
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);
    connect(kristalli, SIGNAL(ConnectionAttemptFailed()), this, SLOT(OnConnectionAttemptFailed()), Qt::UniqueConnection);

    owner_->GetKristalliModule()->Connect(address.toStdString().c_str(), port, protocol);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
}

void Client::Logout()
{
    QTimer::singleShot(1, this, SLOT(DelayedLogout()));
}

void Client::DelayedLogout()
{
    DoLogout(false);
}

void Client::DoLogout(bool fail)
{
    if (loginstate_ != NotConnected)
    {
        if (GetConnection())
        {
            owner_->GetKristalliModule()->Disconnect();
            ::LogInfo("Disconnected");
        }
        
        loginstate_ = NotConnected;
        client_id_ = 0;
        
        framework_->Scene()->RemoveScene("TundraClient");
        framework_->Asset()->ForgetAllAssets();
        
        emit Disconnected();
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
        properties.clear();
    }

    KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
    disconnect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)));

    disconnect(kristalli, SIGNAL(ConnectionAttemptFailed()), this, SLOT(OnConnectionAttemptFailed()));

    ::LogInfo("Client logged out.");
}

bool Client::IsConnected() const
{
    return loginstate_ == LoggedIn;
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
    key = key.trimmed();
    LoginPropertyMap::const_iterator i = properties.find(key);
    if (i != properties.end())
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
    kNet::MessageConnection* connection = GetConnection();
    switch(loginstate_)
    {
    case ConnectionPending:
        if (connection && connection->GetConnectionState() == kNet::ConnectionOK)
        {
            loginstate_ = ConnectionEstablished;
            MsgLogin msg;
            emit AboutToConnect(); // This signal is used as a 'function call'. Any interested party can fill in
            // new content to the login properties of the client object, which will then be sent out on the line below.
            msg.loginData = StringToBuffer(LoginPropertiesAsXml().toStdString());
            connection->Send(msg);
        }
        break;
    case LoggedIn:
        // If we have logged in, but connection dropped, prepare to resend login
        if (!connection || connection->GetConnectionState() != kNet::ConnectionOK)
            loginstate_ = ConnectionPending;
        break;
    }
}

void Client::GetCameraOrientation()
{
    if(!sendCameraUpdates_)
        return;

    if(framework_->IsHeadless())
        return;

    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

    if(!renderer)
        return;

    Entity *parentEntity = renderer->MainCamera();

    if(!parentEntity)
        return;

    EC_Placeable *camera_placeable = parentEntity->GetComponent<EC_Placeable>().get();

    if(!camera_placeable)
        return;

    Quat orientation = camera_placeable->WorldOrientation();
    float3 location = camera_placeable->WorldPosition();

    if(orientation == currentcameraorientation_ && location.Equals(currentcameralocation_)) //If the position and orientation of the client has not changed. Do not send anything
        return;

    const int maxMessageSizeBytes = 1400;

    Ptr(kNet::MessageConnection) connection = GetConnection();

    kNet::NetworkMessage *msg = connection->StartNewMessage(cCameraOrientationUpdate, maxMessageSizeBytes);

    msg->contentID = 0;
    msg->inOrder = true;
    msg->reliable = true;

    kNet::DataSerializer ds(msg->data, maxMessageSizeBytes);

    const Transform &t = camera_placeable->transform.Get();

    //Serialize the position of the client inside the message. Sends 57 bits.
    ds.AddSignedFixedPoint(11, 8, t.pos.x);
    ds.AddSignedFixedPoint(11, 8, t.pos.y);
    ds.AddSignedFixedPoint(11, 8, t.pos.z);

    //Serialize the orientation of the client. Sends 17 bits.
    float3x3 rot = t.Orientation3x3();
    float3 forward = rot.Col(2);
    forward.Normalize();
    ds.AddNormalizedVector3D(forward.x, forward.y, forward.z, 9, 8);

    //Update the current location and orientation of the client.
    currentcameraorientation_ = orientation;
    currentcameralocation_ = location;

    // Finally send the message
    SendCameraOrientation(ds, msg);
}

void Client::SendCameraOrientation(kNet::DataSerializer ds, kNet::NetworkMessage *msg)
{
    Ptr(kNet::MessageConnection) connection = GetConnection();

    if (ds.BytesFilled() > 0)
        connection->EndAndQueueMessage(msg, ds.BytesFilled());

    else
        connection->FreeMessage(msg);
}

kNet::MessageConnection* Client::GetConnection()
{
    return owner_->GetKristalliModule()->GetMessageConnection();
}

void Client::OnConnectionAttemptFailed()
{
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
    DoLogout(true);
}

void Client::HandleKristalliMessage(MessageConnection* source, packet_id_t packetId, message_id_t messageId, const char* data, size_t numBytes)
{
    if (source != GetConnection())
    {
        ::LogWarning("Client: dropping message " + QString::number(messageId) + " from unknown source");
        return;
    }
    
    switch(messageId)
    {
    case MsgCameraOrientationRequest::messageID:
        {
            MsgCameraOrientationRequest msg(data, numBytes);
            HandleCameraOrientationRequest(source, msg);
        }
        break;
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

void Client::HandleCameraOrientationRequest(MessageConnection* source, const MsgCameraOrientationRequest& msg)
{
    sendCameraUpdates_ = msg.enableCameraUpdates;
}

void Client::HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    if (msg.success)
    {
        loginstate_ = LoggedIn;
        client_id_ = msg.userID;
        ::LogInfo("Logged in successfully");
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnect_)
        {
            // Create a non-authoritative scene for the client
            ScenePtr scene = framework_->Scene()->CreateScene("TundraClient", true, false);

//            framework_->Scene()->SetDefaultScene(scene);
            owner_->GetSyncManager()->RegisterToScene(scene);
            
            UserConnectedResponseData responseData;
            if (msg.loginReplyData.size() > 0)
                responseData.responseData.setContent(QByteArray((const char *)&msg.loginReplyData[0], (int)msg.loginReplyData.size()));

            emit Connected(&responseData);

            if(!framework_->IsHeadless())
            {
                cameraUpdateTimer = new QTimer(this);
                connect(cameraUpdateTimer, SIGNAL(timeout()), this, SLOT(GetCameraOrientation()));
                cameraUpdateTimer->start(500);
            }
        }
        else
        {
            // If we are reconnecting, empty the scene, as the server will send everything again anyway
            // Note: when we move to unordered communication, we must guarantee that the server does not send
            // any scene data before the login reply

            ScenePtr scene = framework_->Scene()->GetScene("TundraClient");
            if (scene)
                scene->RemoveAllEntities(true, AttributeChange::LocalOnly);
        }
        reconnect_ = true;
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

}

