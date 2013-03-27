// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "MumbleNetworkHandler.h"
#include "Mumble.pb.h"

#include "celt/celt.h"

#include "CoreTypes.h"
#include "Application.h"
#include "ConsoleAPI.h"
#include "LoggingFunctions.h"

#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QSslCipher>
#include <QMutexLocker>

#ifdef Q_OS_UNIX
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#ifdef Q_OS_WIN

#include "delayimp.h"
#include "Qos2.h"

static HANDLE TryLoadQoS() 
{
    HANDLE handle = NULL;
    HRESULT hr = E_FAIL;

    __try 
    {
        hr = __HrLoadAllImportsForDll("qwave.dll");
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        hr = E_FAIL;
    }

    if (! SUCCEEDED(hr)) 
        LogInfo("[MumbleNetworkHandler]: Failed to load qWave.dll, no QoS available.");
    else 
    {
        QOS_VERSION qvVer;
        qvVer.MajorVersion = 1;
        qvVer.MinorVersion = 0;

        if (!QOSCreateHandle(&qvVer, &handle)) 
        {
            LogWarning("[MumbleNetworkHandler]: Failed to create QOS2 handle.");
            handle = NULL;
        }
    }
    return handle;
}
#endif

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
    networkMode(MumbleNetwork::MumbleUDPMode)
{
    connectionInfo.address = address;
    connectionInfo.port = port;
    connectionInfo.username = username;
    connectionInfo.password = password;
    
#ifdef Q_OS_WIN
    dwFlowTCP = 0;
    dwFlowUDP = 0;
    hQoS = TryLoadQoS();
#endif
}

MumbleNetworkHandler::~MumbleNetworkHandler()
{
    // Cleanup is done in run() when thread exits.
}

void MumbleNetworkHandler::run()
{
    exiting_ = false;

    // Test that the host and port exists. We should not start the
    // encrypted SSL socket connection before we know its safe.
    // Unfortunately this has to be done due to OpenSSL crashing on us via
    // QSslSocket if it does not exist.
    QTcpSocket *socketTester = new QTcpSocket();
    socketTester->connectToHost(connectionInfo.address, connectionInfo.port);
    if (!socketTester->waitForConnected(30000))
    {
        SAFE_DELETE(socketTester);
        emit Disconnected("Connection timed out.");
        quit();
        return;
    }
    socketTester->waitForReadyRead(25);
    socketTester->disconnectFromHost();
    if (socketTester->state() != QAbstractSocket::UnconnectedState)
        socketTester->waitForDisconnected(100);
    if (socketTester->isOpen())
        socketTester->close();
    SAFE_DELETE(socketTester);

    // If false is returned there is no SSL support.
    if (!InitTCP())
    {
        emit Disconnected("SSL not supported, cannot initialize TCP connection.");
        quit();
        return;
    }
    tcp->connectToHostEncrypted(connectionInfo.address, connectionInfo.port);

    timestamp.restart();
    pingTimer_ = new QTimer(this);
    pingTimer_->start(5000);
    connect(pingTimer_, SIGNAL(timeout()), SLOT(SendPing()));

    int qobjTimerId = startTimer(15); // ~60 fps

    exec(); // Blocks untill quit()

    exiting_ = true;
    killTimer(qobjTimerId);   
    pingTimer_->stop();
    
    if (tcp && tcp->state() != QAbstractSocket::UnconnectedState)
    {
        requestedExit_ = true;
        tcp->disconnectFromHost();
        tcp->waitForDisconnected();
        emit Disconnected("Disconnected by users request.");
    }
    else
        emit Disconnected("");

    if (tcp && tcp->isOpen())
        tcp->close();
        
    // Remove QoS
#ifdef Q_OS_WIN
    if (hQoS != NULL)
    {
        if (dwFlowUDP && !QOSRemoveSocketFromFlow(hQoS, 0, dwFlowUDP, 0))
            LogWarning(LC + "Failed to remove UDP flow from QoS.");
        dwFlowUDP = 0;
        if (dwFlowTCP && !QOSRemoveSocketFromFlow(hQoS, 0, dwFlowTCP, 0))
            LogWarning(LC + "Failed to remove TCP flow from QoS.");
        dwFlowTCP = 0;
    }
#endif

    // Note that tcp and udp objects are freed by Qt parenting.
}

void MumbleNetworkHandler::timerEvent(QTimerEvent * /*event*/)
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

bool MumbleNetworkHandler::InitTCP()
{    
    if (!QSslSocket::supportsSsl())
        return false;

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
    tcp->moveToThread(this);

    connect(tcp, SIGNAL(encrypted()), this, SLOT(OnConnected()));
    connect(tcp, SIGNAL(disconnected()), SLOT(OnDisconnected()));
    connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(OnError(QAbstractSocket::SocketError)));
    connect(tcp, SIGNAL(sslErrors(const QList<QSslError>&)), SLOT(OnSslErrors(const QList<QSslError>&)));
    connect(tcp, SIGNAL(readyRead()), this, SLOT(OnTCPSocketRead()));

    return true;
}

bool MumbleNetworkHandler::InitUDP()
{
    QMutexLocker udpLock(&mutexUDP);

    udp = new QUdpSocket(this);
    udp->moveToThread(this);

    udpInfo.host = tcp->peerAddress();
    udpInfo.port = connectionInfo.port;

    if (udpInfo.host.protocol() == QAbstractSocket::IPv6Protocol)
        udp->bind(QHostAddress(QHostAddress::AnyIPv6), 0);
    else
        udp->bind(QHostAddress(QHostAddress::Any), 0);

    connect(udp, SIGNAL(readyRead()), this, SLOT(OnUDPSocketRead()));

    // Set TOS/QoS to describe the UDP socket it is for voice data!
    
#if defined(Q_OS_UNIX)
// If you enable this code, also check out InitUDP() that has similar set of code commented out.
    int val = 0xe0;
    if (setsockopt(udp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val))) 
    {
        val = 0x80;
        if (setsockopt(udp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val)))
            LogWarning(LC + "Failed to set TOS for UDP Socket");
    }
#if defined(SO_PRIORITY) // NOT DEFINED IN TUNDRA, http://linux.die.net/man/7/socket
    socklen_t optlen = sizeof(val);
    if (getsockopt(udp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, &optlen) == 0) 
    {
        if (val == 0) 
        {
            val = 6;
            setsockopt(udp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, sizeof(val));
        }
    }
#endif

#elif defined(Q_OS_WIN)
    if (hQoS != NULL)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(udpInfo.port);
        addr.sin_addr.s_addr = htonl(udpInfo.host.toIPv4Address());
        dwFlowUDP = 0;
        if (!QOSAddSocketToFlow(hQoS, udp->socketDescriptor(), reinterpret_cast<sockaddr*>(&addr), QOSTrafficTypeVoice, QOS_NON_ADAPTIVE_FLOW, &dwFlowUDP))
            LogWarning(LC + "Failed to add UDP flow to QOS.");
    }
#endif
    return true;
}

void MumbleNetworkHandler::OnConnected()
{
    // Set TOS/QoS to describe the TCP socket it's priority!
    
#if defined(Q_OS_WIN)
    // Already initialized or qos not available via qwave.dll delayed loading.
    if (dwFlowTCP || hQoS == NULL)
        return;

    dwFlowTCP = 0;
    if (!QOSAddSocketToFlow(hQoS, tcp->socketDescriptor(), NULL, QOSTrafficTypeAudioVideo, QOS_NON_ADAPTIVE_FLOW, &dwFlowTCP))
        LogWarning(LC + "Failed to add TCP flow to QOS.");
#elif defined(Q_OS_UNIX)
// If you enable this code, also check out InitUDP() that has similar set of code commented out.
    int val = 0xa0;
    if (setsockopt(tcp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val))) {
        val = 0x60;
        if (setsockopt(tcp->socketDescriptor(), IPPROTO_IP, IP_TOS, &val, sizeof(val)))
            LogWarning(LC + "Failed to set TOS for TCP Socket");
    }
#if defined(SO_PRIORITY)
    socklen_t optlen = sizeof(val);
    if (getsockopt(tcp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, &optlen) == 0) {
        if (val == 0) {
            val = 6;
            setsockopt(tcp->socketDescriptor(), SOL_SOCKET, SO_PRIORITY, &val, sizeof(val));
        }
    }

#endif

#endif

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
}

void MumbleNetworkHandler::OnDisconnected()
{
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
    else
        emit Disconnected("TCP socket error.");

    quit();
}

void MumbleNetworkHandler::OnSslErrors(const QList<QSslError>& /*errors*/)
{
    /** In mumble client ssl errors are always ignored as well.
        The only difference is that it pops a "do you accept" dialog where you can inspect the
        certificate. This data is then stored to database so you are not bothered again for that particular server. 
        @todo Implement above procedures. */
    LogDebug("[MumbleNetworkHandler]: SSL errors occurred during handshake. Ignoring for now...");
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
                stream >> seq;

                HandleVoicePacket(userId, seq, stream);
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
    messageFlags |= (UDPVoiceCELTAlpha << 5);

    char data[1024];
    data[0] = static_cast<unsigned char>(messageFlags);

    Mumble::PacketDataStream stream(data + 1, 1023);
    PrepareVoicePacket(packetInfo.encodedFrames, stream);

    if (packetInfo.isPositional)
    {
        stream << packetInfo.pos.x;
        stream << packetInfo.pos.y;
        stream << packetInfo.pos.z;
    }

    NetworkMode localNetworkMode = MumbleUDPMode;
    {
        QMutexLocker modeLock(&mutexNetworkMode);
        localNetworkMode = networkMode;
    }
    if (localNetworkMode == MumbleUDPMode)
        SendUDP(data, stream.size() + 1);
    else
        SendTCP(UDPTunnel, data, stream.size() + 1);
}

void MumbleNetworkHandler::PrepareVoicePacket(ByteArrayVector &encodedFrames, Mumble::PacketDataStream &stream)
{
    // Sequence number
    stream << frameOutSequenceNumber;
    frameOutSequenceNumber++;

    int frameCount = encodedFrames.size();
    for(int i=0; i<frameCount; ++i)
    {
        const QByteArray &qba = encodedFrames.at(i);
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
        case MumbleNetwork::ServerSync:
        {
            MumbleProto::ServerSync msg = ParseMessage<MumbleProto::ServerSync>(buffer);
            connectionInfo.sessionId = msg.session();
            emit ServerSynced(connectionInfo.sessionId);
            break;
        }
        case MumbleNetwork::ServerConfig:
        {
            MumbleProto::ServerConfig msg = ParseMessage<MumbleProto::ServerConfig>(buffer);
            /// @todo Detect max bandwidth and make it as max of our own?
            //if (msg.has_max_bandwidth())
            break;
        }
        case MumbleNetwork::Version:
        {
            MumbleProto::Version msg = ParseMessage<MumbleProto::Version>(buffer);
            LogDebug(LC + "Murmur server information:");
            LogDebug(LC + "- Release: " + utf8(msg.release()));
            LogDebug(LC + "- OS: " + utf8(msg.os()) + " / " + utf8(msg.os_version()));
            break;
        }
        case MumbleNetwork::Reject:
        {
            MumbleProto::Reject msg = ParseMessage<MumbleProto::Reject>(buffer);
            QString reason = msg.has_reason() ? utf8(msg.reason()) : "";
            emit Disconnected(reason);
            emit ConnectionRejected((MumbleNetwork::RejectReason)msg.type(), reason);
            break;
        }
        case MumbleNetwork::ChannelState:
        {
            MumbleProto::ChannelState msg = ParseMessage<MumbleProto::ChannelState>(buffer);
            emit ChannelUpdate(msg.channel_id(), msg.parent(), utf8(msg.name()), msg.has_description() ? utf8(msg.description()) : "");
            break;
        }
        case MumbleNetwork::ChannelRemove:
        {
            MumbleProto::ChannelRemove msg = ParseMessage<MumbleProto::ChannelRemove>(buffer);
            emit ChannelRemoved(msg.channel_id());
            break;
        }
        case MumbleNetwork::UserState:
        {
            MumbleProto::UserState msg = ParseMessage<MumbleProto::UserState>(buffer);

            // Fill in our applications user state struct, we don't want the MumblePlugin layer to
            // know anything about MumbleProto data structures. But we need to know exactly what has changed.
            // Otherwise we will start to get our of sync with the server.
            MumbleNetwork::MumbleUserState userState;
            userState.hasId = msg.has_session();
            userState.hasChannelId = msg.has_channel_id();
            userState.hasName = msg.has_name();
            userState.hasComment = msg.has_comment();
            userState.hasHash = msg.has_hash();
            userState.hasSelfMute = msg.has_self_mute();
            userState.hasSelfDeaf = msg.has_self_deaf();

            if (userState.hasId)
            {
                userState.id = msg.session();
                userState.isMe = connectionInfo.sessionId == userState.id ? true : false;
            }
            if (userState.hasChannelId)
                userState.channelId = msg.channel_id();
            if (userState.hasName)
                userState.name = utf8(msg.name());
            if (userState.hasComment)
                userState.comment = utf8(msg.comment());
            if (userState.hasHash)
                userState.hash = utf8(msg.hash());
            if (userState.hasSelfMute)
                userState.selfMuted = msg.self_mute();
            if (userState.hasSelfDeaf)
                userState.selfDeaf = msg.self_deaf();

            emit UserUpdate(userState);
            break;
        }
        case MumbleNetwork::UserRemove:
        {
            MumbleProto::UserRemove msg = ParseMessage<MumbleProto::UserRemove>(buffer);
            emit UserLeft(msg.session(), msg.actor(), msg.has_ban() ? msg.ban() : false, msg.has_ban() ? !msg.ban() : false, utf8(msg.reason()));
            break;
        }
        case MumbleNetwork::UDPTunnel:
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

                HandleVoicePacket(userId, seq, stream);
            }
            break;
        }
        case MumbleNetwork::CryptSetup:
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
        case MumbleNetwork::Ping:
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
                emit NetworkModeChange(networkMode, "UDP packets can be sent to and received from the server. Switching to UDP mode.");
            }
            break;
        }
        case MumbleNetwork::PermissionDenied:
        {
            MumbleProto::PermissionDenied message = ParseMessage<MumbleProto::PermissionDenied>(buffer);
            emit PermissionDenied((MumbleNetwork::PermissionDeniedType)message.type(), (MumbleNetwork::ACLPermission)message.permission(),
                message.has_channel_id() ? message.channel_id() : 0, message.has_session() ? message.session() : 0, message.has_reason() ? utf8(message.reason()) : "");
            break;
        }
        case MumbleNetwork::TextMessage:
        {
            MumbleProto::TextMessage message = ParseMessage<MumbleProto::TextMessage>(buffer);
            if (message.has_actor() && message.has_message())
            {
                QList<uint> userIds;
                for(int i=0; i<message.session_size(); i++)
                    userIds.push_back(message.session(i));
                if (!userIds.isEmpty())
                {
                    if (userIds.contains(connectionInfo.sessionId))
                        emit TextMessageReceived(true, QList<uint>(), message.actor(), utf8(message.message()));
                }
                else
                {
                    QList<uint> channelIds;
                    for(int i=0; i<message.channel_id_size(); i++)
                        channelIds.push_back(message.channel_id(i));
                    if (!channelIds.isEmpty())
                        emit TextMessageReceived(false, channelIds, message.actor(), utf8(message.message()));
                }
            }
            break;
        }
        case MumbleNetwork::PermissionQuery:
        {
            /// @todo Check permissions if we have the right to speak/write messages, not terribly important in Tundra right now.
            break;
        }
        case MumbleNetwork::CodecVersion:
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

void MumbleNetworkHandler::HandleVoicePacket(uint userId, uint seq, Mumble::PacketDataStream &stream)
{
    // Read audio frames
    ByteArrayVector frames;
    bool lastFrame = false;
    while(!lastFrame && stream.isValid())
    {
        u8 header = stream.next8();
        uint frameSize = header & 0x7f;
        lastFrame = !(header & 0x80);

        if (frameSize > 0)
            frames.push_back(QByteArray(stream.charPtr(), frameSize));
        stream.skip(frameSize);
    }

    // Check and read positional data
    bool isPositional = false;
    float3 pos = float3::zero;
    if (stream.left() > 0)
    {
        isPositional = true;
        stream >> pos.x;
        stream >> pos.y;
        stream >> pos.z;
    }

    if (frames.size() > 0)
        emit AudioReceived(userId, seq, frames, isPositional, pos);
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
