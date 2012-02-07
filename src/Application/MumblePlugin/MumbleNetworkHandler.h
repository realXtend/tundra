// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "MumbleDefines.h"
#include "MumbleNetwork.h"
#include "mumble/CryptState.h"
#include "mumble/Timer.h"
#include "mumble/PacketDataStream.h"

#include "Math/float3.h"

#include <QObject>
#include <QSslSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
#include <QMutex>

class MumbleNetworkHandler : public QThread
{

Q_OBJECT

public:
    MumbleNetworkHandler(QString address, ushort port, QString username, QString password);
    virtual ~MumbleNetworkHandler();

    MumbleNetwork::ConnectionInfo connectionInfo;
    int codecBitStreamVersion;

protected:
    // QThread override.
    void run();

    // QObject override.
    void timerEvent(QTimerEvent *event);

public slots:
    // Send TCP message with id and message content.
    void SendTCP(const MumbleNetwork::TCPMessageType id, MumbleNetwork::MumbleMessage &message);

    // Send TCP message with id, data ptr and length.
    void SendTCP(const MumbleNetwork::TCPMessageType id, const char *data, int length);

    // Send UDP message with data and length. Encrypts the input data before sending.
    void SendUDP(const char *data, int length);

    // Sends UDP voice packets after processing audio frames and position.
    void SendVoicePacket(MumbleNetwork::VoicePacketInfo &packetInfo);

    // Send TCP and UDP ping.
    void SendPing();

    // Handles TCP messages of id with buffer.
    void HandleMessage(const MumbleNetwork::TCPMessageType id, QByteArray &buffer);

signals:
    void Connected(QString address, int port, QString username);
    void Disconnected(QString reason);
    
    void StateChange(MumbleNetwork::ConnectionState newState);
    void ServerSynced(uint sessionId);

    void NetworkModeChange(MumbleNetwork::NetworkMode mode, QString reason);

    void TextMessageReceived(bool isPrivate, QList<uint> channelIds, uint senderId, QString message);

    void ConnectionRejected(MumbleNetwork::RejectReason reasonType, QString reasonMessage);
    void PermissionDenied(MumbleNetwork::PermissionDeniedType denyType, MumbleNetwork::ACLPermission permission, uint channelId, uint targetUserId, QString reason);

    void ChannelUpdate(uint id, uint parentId, QString name, QString description);
    void ChannelRemoved(uint id);
    void UserUpdate(uint id, uint channelId, QString name, QString comment, QString hash, bool selfMuted, bool selfDeaf, bool isMe);
    void UserLeft(uint id, uint actorId, bool banned, bool kicked, QString reason);
    
    void AudioReceived(uint userId, uint seq, ByteArrayVector frames, bool isPositional, float3 pos);

private slots:
    void OnConnected();
    void OnDisconnected();
    void OnError(QAbstractSocket::SocketError socketError);
    void OnSslErrors(const QList<QSslError>& errors);
    
    void OnTCPSocketRead();
    void OnUDPSocketRead();

private:
    bool InitTCP();
    bool InitUDP();
    
    bool TCPAlive();
    bool UDPAlive();

    // Handle incoming voice data stream from user. Handles both TCP and
    // UDP voice traffic after initial header information is parsed out
    // in their respective handlers.
    void HandleVoicePacket(uint userId, uint seq, Mumble::PacketDataStream &stream);

    // Prepares encoded packets into a PacketDataStream.
    void PrepareVoicePacket(ByteArrayVector &encodedFrames, Mumble::PacketDataStream &stream);

    QSslSocket *tcp;
    QUdpSocket *udp;

    MumbleNetwork::NetworkMode networkMode;

    MumbleNetwork::TCPInfo tcpInfo;
    MumbleNetwork::UDPInfo udpInfo;

    Mumble::CryptState crypt;
    Mumble::Timer timestamp;
    
    QTimer *pingTimer_;

    bool exiting_;
    bool requestedExit_;

    QMutex mutexPendingTCP;
    QMutex mutexUDP;
    QMutex mutexNetworkMode;

    MumbleNetwork::PendingMessageList pedingTCP;

    int frameOutSequenceNumber;

    QString LC;
};
