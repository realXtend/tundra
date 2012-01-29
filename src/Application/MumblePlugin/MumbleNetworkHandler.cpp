// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "MumbleNetworkHandler.h"
#include "mumble/SSL.h"
#include "Mumble.pb.h"

#include "celt/celt.h"

#include "CoreTypes.h"
#include "Application.h"
#include "ConsoleAPI.h"
#include "LoggingFunctions.h"

#include <QByteArray>
#include <QDataStream>
#include <QSslCipher>
#include <QMutexLocker>

using namespace MumbleNetwork;

MumbleNetworkHandler::MumbleNetworkHandler(QString address, ushort port, QString username, QString password) :
    LC("[MumbleNetworkHandler]: "),
    tcp(0),
    udp(0),
    pingTimer_(0),
    exiting_(false),
    requestedExit_(false),
    codecBitStreamVersion(0),
    frameOutSequenceNumber(0),
    networkMode(MumbleNetwork::MumbleTCPMode)
{
    connectionInfo.address = address;
    connectionInfo.port = port;
    connectionInfo.username = username;
    connectionInfo.password = password;
}

MumbleNetworkHandler::~MumbleNetworkHandler()
{
}

void MumbleNetworkHandler::run()
{
    exiting_ = false;
    emit StateChange(MumbleConnecting);

    InitTCP();
    tcp->connectToHostEncrypted(connectionInfo.address, connectionInfo.port);

    timestamp.restart();
    pingTimer_ = new QTimer(this);
    pingTimer_->start(5000);
    connect(pingTimer_, SIGNAL(timeout()), SLOT(SendPing()));

    int qobjTimerId = startTimer(1);

    exec(); // Blocks untill quit()

    killTimer(qobjTimerId);

    exiting_ = true;
    pingTimer_->stop();

    if (tcp->state() != QSslSocket::UnconnectedState)
    {
        requestedExit_ = true;
        tcp->disconnectFromHost();
        tcp->waitForDisconnected();
    }

    emit StateChange(MumbleDisconnected);
}

void MumbleNetworkHandler::timerEvent(QTimerEvent *event)
{
    QMutexLocker tcpPendingLock(&mutexPendingTCP);
    if (!pedingTCP.empty())
    {
        PendingMessageList::const_iterator iterTCP = pedingTCP.begin();
        PendingMessageList::const_iterator endTCP = pedingTCP.end();
        while(iterTCP != endTCP)
        {
            const QByteArray &tcpData = (*iterTCP).data;
            if (tcp->write(tcpData) == -1)
                LogError(LC + "Failed to send main thread initialized TCP message.");
            iterTCP++;
        }
        pedingTCP.clear();
    }
}

void MumbleNetworkHandler::InitTCP()
{    
    if (!QSslSocket::supportsSsl())
    {
        LogError(LC + "SSL not supported, cannot initialize TCP connection!");
        return;
    }

    QStringList addedCerts = Mumble::MumbleSSL::addSystemCA();
    foreach(QString certMsg, addedCerts)
        LogInfo(LC + certMsg);

    QList<QSslCipher> pref;
    foreach(QSslCipher c, QSslSocket::defaultCiphers()) 
    {
        if (c.usedBits() < 128)
            continue;
        pref << c;
    }
    if (!pref.isEmpty())
        QSslSocket::setDefaultCiphers(pref);
    else
        LogWarning(LC + "No ciphers of at least 128 bit found");

    tcp = new QSslSocket(this);
    tcp->setProtocol(QSsl::TlsV1);

    connect(tcp, SIGNAL(encrypted()), this, SLOT(OnConnected()));
    connect(tcp, SIGNAL(disconnected()), SLOT(OnDisconnected()));
    connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(OnError(QAbstractSocket::SocketError)));
    connect(tcp, SIGNAL(sslErrors(const QList<QSslError>&)), SLOT(OnSslErrors(const QList<QSslError>&)));
    connect(tcp, SIGNAL(readyRead()), this, SLOT(OnTCPSocketRead()));
}

void MumbleNetworkHandler::InitUDP()
{
    QMutexLocker udpLock(&mutexUDP);

    udp = new QUdpSocket(this);
    
    udpInfo.host = tcp->peerAddress();
    udpInfo.port = connectionInfo.port;

    if (udpInfo.host.protocol() == QAbstractSocket::IPv6Protocol)
        udp->bind(QHostAddress(QHostAddress::AnyIPv6), 0);
    else
        udp->bind(QHostAddress(QHostAddress::Any), 0);

    connect(udp, SIGNAL(readyRead()), this, SLOT(OnUDPSocketRead()));

    /// @todo Checkout TOS/QoS later to describe the UDP channel is for voice data!
    /// http://msdn.microsoft.com/en-us/library/windows/desktop/aa374027(v=vs.85).aspx
    
#if defined(Q_OS_UNIX)
/*
    int val = 0xe0;
    if (setsockopt(qusUdp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val))) 
    {
        val = 0x80;
        if (setsockopt(qusUdp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val)))
            qWarning("ServerHandler: Failed to set TOS for UDP Socket");
    }
#if defined(SO_PRIORITY) // NOT DEFINED IN TUNDRA
    socklen_t optlen = sizeof(val);
    if (getsockopt(qusUdp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, &optlen) == 0) 
    {
        if (val == 0) 
        {
            val = 6;
            setsockopt(qusUdp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, sizeof(val));
        }
    }
#endif
*/
#elif defined(Q_OS_WIN)
/*
    if (hQoS != NULL) 
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(usPort);
        addr.sin_addr.s_addr = htonl(qhaRemote.toIPv4Address());
        dwFlowUDP = 0;
        if (!QOSAddSocketToFlow(hQoS, qusUdp->socketDescriptor(), reinterpret_cast<sockaddr*>(&addr), QOSTrafficTypeVoice, QOS_NON_ADAPTIVE_FLOW, &dwFlowUDP))
            qWarning("ServerHandler: Failed to add UDP to QOS");
    }
*/
#endif
}

void MumbleNetworkHandler::OnConnected()
{
    LogInfo(LC + "OnConnected");

    MumbleProto::Version messageVersion;
    messageVersion.set_release(utf8(Application::FullIdentifier()));
    messageVersion.set_os(utf8(Application::Platform()));
    messageVersion.set_version(MUMBLE_VERSION(1, 2, 3));
    SendTCP(Version, messageVersion);

    MumbleProto::Authenticate messageAuth;
    messageAuth.set_username(utf8(connectionInfo.username));
    messageAuth.set_password(utf8(connectionInfo.password));
    messageAuth.add_celt_versions(codecBitStreamVersion);
    messageAuth.set_opus(false);
    SendTCP(Authenticate, messageAuth);

    InitUDP();

    emit Connected(connectionInfo.address, (int)connectionInfo.port, connectionInfo.username);
    emit StateChange(MumbleConnected);
}

void MumbleNetworkHandler::OnDisconnected()
{
    if (requestedExit_)
        emit Disconnected("Disconnected by users request.");
    emit StateChange(MumbleDisconnected);

    if (!exiting_)
        quit();
}

void MumbleNetworkHandler::OnError(QAbstractSocket::SocketError socketError)
{
    if (socketError == QAbstractSocket::ConnectionRefusedError)
        emit Disconnected("Connection refused.");
    else if (socketError == QAbstractSocket::SslHandshakeFailedError)
        emit Disconnected("SSL handshake failed.");
    else if (socketError == QAbstractSocket::RemoteHostClosedError)
        emit Disconnected("Server closed connection.");
}

void MumbleNetworkHandler::OnSslErrors(const QList<QSslError>& errors)
{
    /** In mumble client ssl errors are always ignored as well.
        The only difference is that it pops a "do you accept" dialog where you can inspect the
        certificate. This data is then stored to database so you are not bothered again for that particular server. 
        @todo Implement above procedures. */
    LogWarning("[MumbleNetworkHandler]: SSL errors occurred during handshake. Ignoring for now...");
    tcp->ignoreSslErrors();
}

void MumbleNetworkHandler::OnTCPSocketRead()
{
    while (true) 
    {
        quint16 messageType = 0;
        qint64 availableBytes = tcp->bytesAvailable();
        if (tcpInfo.messageLength == -1) 
        {
            if (availableBytes < 6)
                return;

            unsigned char headerBuffer[6];
            tcp->read(reinterpret_cast<char *>(headerBuffer), 6);
            messageType = qFromBigEndian<quint16>(&headerBuffer[0]);
            tcpInfo.messageLength = qFromBigEndian<quint32>(&headerBuffer[2]);

            availableBytes -= 6;
        }

        // If the full message is not available, wait
        if ((tcpInfo.messageLength == -1) || (availableBytes < tcpInfo.messageLength))
            return;

        // Read full message from the socket
        QByteArray buffer = tcp->read(tcpInfo.messageLength);
        tcpInfo.messageLength = -1;
        availableBytes -= tcpInfo.messageLength;

        HandleMessage((TCPMessageType)messageType, buffer);
    }
}

void MumbleNetworkHandler::OnUDPSocketRead()
{
    QMutexLocker udpLock(&mutexUDP);

    while (udp->hasPendingDatagrams()) 
    {
        char encrypted[2048];
        char buffer[2048];
        unsigned int buflen = static_cast<unsigned int>(udp->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;
        udp->readDatagram(encrypted, qMin(2048U, buflen), &senderAddress, &senderPort);

        // if we are the sender or port is incorrect
        if (!(udpInfo.host == senderAddress) || (connectionInfo.port != senderPort))
        {
            LogError(LC + "sender address or port invalid!");
            continue;
        }

        // Crypt check
        if (!crypt.isValid())
        {
            LogError(LC + "Crypt not valid");
            continue;
        }
        if (buflen < 5)
            continue;

        // Decrypt
        if (!crypt.decrypt(reinterpret_cast<const unsigned char *>(encrypted), reinterpret_cast<unsigned char *>(buffer), buflen)) 
        {
            if (crypt.tLastGood.elapsed() > 5000000ULL) 
            {
                if (crypt.tLastRequest.elapsed() > 5000000ULL) 
                {
                    crypt.tLastRequest.restart();
                    MumbleProto::CryptSetup message;
                    SendTCP(CryptSetup, message);
                }
            }
            continue;
        }

        // Process data
        Mumble::PacketDataStream stream(buffer + 1, buflen - 5);
        UDPMessageType messageType = static_cast<UDPMessageType>((buffer[0] >> 5) & 0x7);
        //unsigned int messageFlags = buffer[0] & 0x1f;

        switch (messageType) 
        {
            case UDPVoiceCELTAlpha:
            case UDPVoiceCELTBeta:
            {
                uint userId = 0;
                uint seq = 0;
                stream >> userId;
                stream >> seq; /// @todo detect if we have received > seq and ignore packet?

                HandleVoicePacket(userId, stream);
                break;
            }
            case UDPPing:
            {
                // No op?
                break;
            }
            case UDPVoiceOpus:
            case UDPVoiceSpeex:
            {
                // No op in Tundra, we only support celt audio
                break;
            }
            default:
                break;
        }
    }
}

void MumbleNetworkHandler::SendPing()
{
    quint64 time = timestamp.elapsed();

    // UDP ping
    if (UDPAlive())
    {
        unsigned char buffer[256];
        Mumble::PacketDataStream stream(buffer + 1, 255);
        buffer[0] = UDPPing << 5;
        stream << time;
        SendUDP(reinterpret_cast<const char *>(buffer), stream.size() + 1);
    }
    
    // TCP ping
    if (TCPAlive())
    {
        MumbleProto::Ping message;
        message.set_timestamp(time);
        message.set_good(crypt.uiGood);
        message.set_late(crypt.uiLate);
        message.set_lost(crypt.uiLost);
        message.set_resync(crypt.uiResync);
        SendTCP(Ping, message);
    }
}

void MumbleNetworkHandler::SendTCP(const TCPMessageType id, MumbleMessage &message)
{
    if (!TCPAlive())
        return;

    int length = message.ByteSize();
    if (length > 0x7fffff)
        return;

    // Write
    QByteArray ba;
    ba.resize(length + 6);
    unsigned char *buffer = reinterpret_cast<unsigned char *>(ba.data());
    qToBigEndian<quint16>(id, &buffer[0]);
    qToBigEndian<quint32>(length, &buffer[2]);
    message.SerializeToArray(buffer + 6, length);

    if (QThread::currentThread() != thread())
    {
        QMutexLocker tcpPendingLock(&mutexPendingTCP);
        pedingTCP.push_back(PendingMessage(ba));
        return;
    }

    if (tcp->write(ba) == -1)
        LogError(LC + "Failed to send TCP message " + QString::number(id));
}

void MumbleNetworkHandler::SendTCP(const TCPMessageType id, const char *data, int length)
{
    if (!TCPAlive())
        return;

    // Write
    QByteArray ba;
    ba.resize(length + 6);
    unsigned char *buffer = reinterpret_cast<unsigned char *>(ba.data());
    qToBigEndian<quint16>(id, &buffer[0]);
    qToBigEndian<quint32>(length, &buffer[2]);

    memcpy(buffer + 6, data, length);

    if (QThread::currentThread() != thread())
    {
        QMutexLocker tcpPendingLock(&mutexPendingTCP);
        pedingTCP.push_back(PendingMessage(ba));
        return;
    }

    if (tcp->write(ba) == -1)
        LogError(LC + "Failed to send TCP message " + QString::number(id));
}

void MumbleNetworkHandler::SendUDP(const char *data, int length)
{
    if (!UDPAlive())
        return;

    QMutexLocker udpLock(&mutexUDP);

    MUMBLE_STACKVAR(unsigned char, crypto, length + 4);

    crypt.encrypt(reinterpret_cast<const unsigned char *>(data), crypto, length);
    udp->writeDatagram(reinterpret_cast<const char *>(crypto), length + 4, udpInfo.host, udpInfo.port);
}

void MumbleNetworkHandler::SendVoicePacket(VoicePacketInfo &packetInfo)
{
    int messageFlags = 0;
    if (packetInfo.isLoopBack)
        messageFlags = 0x1f;
    messageFlags |= (UDPMessageType::UDPVoiceCELTAlpha << 5);

    char data[1024];
    data[0] = static_cast<unsigned char>(messageFlags);

    Mumble::PacketDataStream stream(data + 1, 1023);
    PrepareVoicePacket(packetInfo.encodedFrames, stream);

    if (packetInfo.isPositional)
    {
        // Tundra to Mumble coordinate conversion
        stream << packetInfo.pos.y;
        stream << packetInfo.pos.z;
        stream << -packetInfo.pos.x;
    }

    NetworkMode localNetworkMode = MumbleTCPMode;
    {
        QMutexLocker modeLock(&mutexNetworkMode);
        localNetworkMode = networkMode;
    }
    if (localNetworkMode == MumbleUDPMode)
        SendUDP(data, stream.size() + 1);
    else
        SendTCP(UDPTunnel, data, stream.size() + 1);
}

void MumbleNetworkHandler::PrepareVoicePacket(QList<QByteArray> &encodedFrames, Mumble::PacketDataStream &stream)
{
    // Sequence number
    stream << frameOutSequenceNumber;
    frameOutSequenceNumber++;

    int frameCount = encodedFrames.count();
    for (int i=0; i<frameCount; ++i) 
    {
        const QByteArray &qba = encodedFrames.takeFirst();
        unsigned char head = static_cast<unsigned char>(qba.size());
        if (i < frameCount - 1)
            head |= 0x80;
        stream.append(head);
        stream.append(qba.constData(), qba.size());
    }
}

void MumbleNetworkHandler::HandleMessage(const TCPMessageType id, QByteArray &buffer)
{
    switch(id)
    {
        case ServerSync:
        {
            MumbleProto::ServerSync msg = ParseMessage<MumbleProto::ServerSync>(buffer);
            connectionInfo.sessionId = msg.session();
            emit ServerSynced(connectionInfo.sessionId);
            break;
        }
        case ServerConfig:
        {
            MumbleProto::ServerConfig msg = ParseMessage<MumbleProto::ServerConfig>(buffer);
            /// @todo Detect max bandwidth and make it as max of our own?
            //if (msg.has_max_bandwidth())
            break;
        }
        case Version:
        {
            MumbleProto::Version msg = ParseMessage<MumbleProto::Version>(buffer);
            LogDebug(LC + "Murmur server information:");
            LogDebug(LC + "- Release: " + utf8(msg.release()));
            LogDebug(LC + "- OS: " + utf8(msg.os()) + " / " + utf8(msg.os_version()));
            break;
        }
        case Reject:
        {
            MumbleProto::Reject msg = ParseMessage<MumbleProto::Reject>(buffer);
            emit ConnectionRejected((MumbleNetwork::RejectReason)msg.type(), msg.has_reason() ? utf8(msg.reason()) : "");
            break;
        }
        case ChannelState:
        {
            MumbleProto::ChannelState msg = ParseMessage<MumbleProto::ChannelState>(buffer);
            emit ChannelUpdate(msg.channel_id(), msg.parent(), utf8(msg.name()), msg.has_description() ? utf8(msg.description()) : "");
            break;
        }
        case ChannelRemove:
        {
            MumbleProto::ChannelRemove msg = ParseMessage<MumbleProto::ChannelRemove>(buffer);
            emit ChannelRemoved(msg.channel_id());
            break;
        }
        case UserState:
        {
            MumbleProto::UserState msg = ParseMessage<MumbleProto::UserState>(buffer);
            emit UserUpdate(msg.session(), msg.channel_id(), utf8(msg.name()), utf8(msg.comment()), utf8(msg.hash()), 
                            msg.self_mute(), msg.self_deaf(), (connectionInfo.sessionId == msg.session() ? true : false));
            break;
        }
        case UserRemove:
        {
            MumbleProto::UserRemove msg = ParseMessage<MumbleProto::UserRemove>(buffer);
            emit UserLeft(msg.session(), msg.actor(), msg.has_ban() ? msg.ban() : false, msg.has_ban() ? !msg.ban() : false, utf8(msg.reason()));
            break;
        }
        case UDPTunnel:
        {
            if (buffer.length() < 1)
                return;

            Mumble::PacketDataStream stream(buffer.constData(), buffer.length());
            u8 firstByte = stream.next8();
            UDPMessageType messageType = static_cast<UDPMessageType>((firstByte >> 5) & 0x07);
            if (messageType == UDPVoiceCELTAlpha || messageType == UDPVoiceCELTBeta)
            {
                uint userId = 0;
                uint seq = 0;
                stream >> userId;
                stream >> seq;

                HandleVoicePacket(userId, stream);
            }
            break;
        }
        case CryptSetup:
        {
            MumbleProto::CryptSetup msg = ParseMessage<MumbleProto::CryptSetup>(buffer);
            if (msg.has_key() && msg.has_client_nonce() && msg.has_server_nonce()) 
            {
                const std::string &key = msg.key();
                const std::string &client_nonce = msg.client_nonce();
                const std::string &server_nonce = msg.server_nonce();
                if (key.size() == AES_BLOCK_SIZE && client_nonce.size() == AES_BLOCK_SIZE && server_nonce.size() == AES_BLOCK_SIZE)
                    crypt.setKey(reinterpret_cast<const unsigned char *>(key.data()), reinterpret_cast<const unsigned char *>(client_nonce.data()), reinterpret_cast<const unsigned char *>(server_nonce.data()));
            } 
            else if (msg.has_server_nonce()) 
            {
                const std::string &server_nonce = msg.server_nonce();
                if (server_nonce.size() == AES_BLOCK_SIZE) 
                {
                    crypt.uiResync++;
                    memcpy(crypt.decrypt_iv, server_nonce.data(), AES_BLOCK_SIZE);
                }
            } 
            else 
            {
                MumbleProto::CryptSetup messageCrypt;
                messageCrypt.set_client_nonce(std::string(reinterpret_cast<const char *>(crypt.encrypt_iv), AES_BLOCK_SIZE));
                SendTCP(CryptSetup, messageCrypt);
            }
        }
        case Ping:
        {
            MumbleProto::Ping msg = ParseMessage<MumbleProto::Ping>(buffer);
            crypt.uiRemoteGood = msg.good();
            crypt.uiRemoteLate = msg.late();
            crypt.uiRemoteLost = msg.lost();
            crypt.uiRemoteResync = msg.resync();

            if (networkMode == MumbleNetwork::MumbleUDPMode && ((crypt.uiRemoteGood == 0) || (crypt.uiGood == 0)) && (timestamp.elapsed() > 20000000ULL)) 
            {
                {
                    QMutexLocker modeLock(&mutexNetworkMode);
                    networkMode = MumbleNetwork::MumbleTCPMode;
                }

                QString networkModeMessage;
                if ((crypt.uiRemoteGood == 0) && (crypt.uiGood == 0))
                    networkModeMessage = "UDP packets cannot be sent to or received from the server. Switching to TCP mode.";
                else if (crypt.uiRemoteGood == 0)
                    networkModeMessage = "UDP packets cannot be sent to the server. Switching to TCP mode.";
                else
                    networkModeMessage = "UDP packets cannot be received from the server. Switching to TCP mode.";
                emit NetworkModeChange(networkMode, networkModeMessage);
            } 
            else if (networkMode != MumbleNetwork::MumbleUDPMode && (crypt.uiRemoteGood > 3) && (crypt.uiGood > 3)) 
            {
                {
                    QMutexLocker modeLock(&mutexNetworkMode);
                    networkMode = MumbleNetwork::MumbleUDPMode;
                }
                emit NetworkModeChange(networkMode, "UDP packets can be sent to and received from the server. Switching back to UDP mode.");
            }
            break;
        }
        case PermissionQuery:
        {
            /// @todo Check permissions if we have the right to speak/write messages, not terribly important in Tundra right now.
            break;
        }
        case CodecVersion:
        {
            // Ignored because we only have one version of celt codec in Tundra.
            // You must have >= 1.2.2 murmur server to have proper codecs for VOIP to work.
            break;
        }
        default:
        {
            LogWarning("Unhandled incoming TCP message " + QString::number(id));
            break;
        }
    }
}

void MumbleNetworkHandler::HandleVoicePacket(uint userId, Mumble::PacketDataStream &stream)
{
    QList<QByteArray> frames;

    bool lastFrame = false;
    while(!lastFrame && stream.isValid())
    {
        u8 header = stream.next8();
        uint frameSize = header & 0x7f;
        lastFrame = !(header & 0x80);

        if (frameSize > 0)
            frames << QByteArray(stream.charPtr(), frameSize);
        stream.skip(frameSize);
    }

    emit AudioReceived(userId, frames);
}

bool MumbleNetworkHandler::TCPAlive()
{
    if (exiting_)
        return false;
    else
        return tcp->state() != QSslSocket::UnconnectedState;
}

bool MumbleNetworkHandler::UDPAlive()
{
    if (exiting_)
        return false;
    else
        return true;
}
