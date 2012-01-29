// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleFwd.h"
#include "CoreTypes.h"
#include "Math/float3.h"
#include "google/protobuf/message.h"

#include <QString>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QHostAddress>

namespace MumbleNetwork
{
    typedef ::google::protobuf::Message MumbleMessage;

    enum ConnectionState
    {
        MumbleConnecting,
        MumbleConnected,
        MumbleDisconnected
    };

    enum NetworkMode
    {
        NetworkModeNotSet,
        MumbleUDPMode,
        MumbleTCPMode
    };

    enum UDPMessageType
    {
        UDPVoiceCELTAlpha,
        UDPPing,
        UDPVoiceSpeex,
        UDPVoiceCELTBeta,
        UDPVoiceOpus
    };

    enum TCPMessageType 
    {
        Version,
        UDPTunnel,
        Authenticate,
        Ping,
        Reject,
        ServerSync,
        ChannelRemove,
        ChannelState,
        UserRemove,
        UserState,
        BanList,
        TextMessage,
        PermissionDenied,
        ACL,
        QueryUsers,
        CryptSetup,
        ContextActionAdd,
        ContextAction,
        UserList,
        VoiceTarget,
        PermissionQuery,
        CodecVersion,
        UserStats,
        RequestBlob,
        ServerConfig
    };

    enum RejectReason 
    {
        RejectReasonNone = 0,
        RejectReasonWrongVersion = 1,
        RejectReasonInvalidUsername = 2,
        RejectReasonWrongUserPW = 3,
        RejectReasonWrongServerPW = 4,
        RejectReasonUsernameInUse = 5,
        RejectReasonServerFull = 6,
        RejectReasonNoCertificate = 7
    };

    class MumbleHeader 
    {
    public:
        MumbleHeader(TCPMessageType id, s32 len)
        {
            SetType(id);
            SetLength(len);
        }

        s16 Type() const 
        { 
            return (data[0] << 8) | data[1]; 
        }
        
        s32 Length() const 
        { 
            return (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5]; 
        }

        const uchar* Data() const 
        { 
            return data; 
        }

        void SetType(TCPMessageType id)
        {
            SetType(static_cast<s16>(id));
        }

        void SetType(s16 type) 
        { 
            data[0] = type >> 8; 
            data[1] = type & 0xFF; 
        }

        void SetLength(s32 len) 
        {
            data[2] = static_cast<uchar>(len >> 24);
            data[3] = static_cast<uchar>(len >> 16);
            data[4] = static_cast<uchar>(len >> 8);
            data[5] = static_cast<uchar>(len & 0xFF);
        }

        void SerializeToStream(QDataStream *stream)
        {
            stream->writeRawData((const char*)&data[0], 6);
        }

        uchar data[6];
    };

    template <class T> 
    T ParseMessage(QByteArray &buffer) 
    {
        T protoBufMessage;
        protoBufMessage.ParseFromArray(buffer.data(), buffer.size());
        return protoBufMessage;
    }

    template <class T>
    T CopyMessage(const T &message)
    {
        T protoBufMessage;
        protoBufMessage.CopyFrom(message);
        return protoBufMessage;
    }

    inline s32 MUMBLE_VERSION(s16 major, s16 minor, s16 patch) 
    {
        return (major << 16) | (minor << 8) | (patch & 0xFF);
    }

    inline std::string utf8(const QString &str) 
    {
        const QByteArray &ba = str.toUtf8();
        return std::string(ba.constData(), ba.length());
    }

    inline QString utf8(const std::string &str) 
    {
        return QString::fromUtf8(str.data(), static_cast<int>(str.length()));
    }

    struct VoicePacketInfo
    {
        VoicePacketInfo(QList<QByteArray> encodedFrames_) 
        {
            isLoopBack = false;
            isPositional = false;
            encodedFrames = encodedFrames_;
            pos = float3::zero;
        }

        bool isLoopBack;
        bool isPositional;
        float3 pos;

        QList<QByteArray> encodedFrames;
    };

    struct TCPInfo
    {
        TCPInfo() { Reset(); }
        void Reset() { messageLength = -1; }

        int messageLength;
    };

    struct UDPInfo
    {
        UDPInfo() { Reset(); }
        void Reset() { }

        QHostAddress host;
        ushort port;
    };

    struct ConnectionInfo
    {
        ConnectionInfo() { Reset(); }
        void Reset() { sessionId = 0; }

        QString address;
        QString username;
        QString password;
        ushort port;
        u32 sessionId;
    };

    struct PendingMessage
    {
        PendingMessage(QByteArray data_) : data(data_.data(), data_.size()) {}
        QByteArray data;
    };

    typedef std::list<MumbleNetwork::PendingMessage> PendingMessageList;

}
