// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleFwd.h"
#include "MumbleDefines.h"
#include "CoreTypes.h"
#include "Math/float3.h"

#include "google/protobuf/message.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QHostAddress>

/// @cond PRIVATE
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

    enum PermissionDeniedType 
    {
        PermissionDeniedText = 0,
        PermissionDeniedPermission = 1,
        PermissionDeniedSuperUser = 2,
        PermissionDeniedChannelName = 3,
        PermissionDeniedTextTooLong = 4,
        PermissionDeniedH9K = 5,
        PermissionDeniedTemporaryChannel = 6,
        PermissionDeniedMissingCertificate = 7,
        PermissionDeniedUserName = 8,
        PermissionDeniedChannelFull = 9
    };

    enum ACLPermission 
    {
        ACLNone = 0x0,
        ACLWrite = 0x1,
        ACLTraverse = 0x2,
        ACLEnter = 0x4,
        ACLSpeak = 0x8,
        ACLMuteDeafen = 0x10,
        ACLMove = 0x20,
        ACLMakeChannel = 0x40,
        ACLLinkChannel = 0x80,
        ACLWhisper = 0x100,
        ACLTextMessage = 0x200,
        ACLMakeTempChannel = 0x400,

        // Root channel only
        ACLKick = 0x10000,
        ACLBan = 0x20000,
        ACLRegister = 0x40000,
        ACLSelfRegister = 0x80000,

        ACLCached = 0x8000000,
        ACLAll = 0xf07ff
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

    struct MumbleUserState
    {
        MumbleUserState()
        {
            hasId = false;
            hasChannelId = false;
            hasName = false;
            hasComment = false;
            hasHash = false;
            hasSelfMute = false;
            hasSelfDeaf = false;
            
            id = 0;
            channelId =0;
            selfMuted = false;
            selfDeaf = false;
            isMe = false;
        }

        void Merge(const MumbleUserState &other)
        {
            if (id != other.id)
                return;

            // Might see tedious but we must be able to merge the states.
            // This way changes wont be lost if a state is pending to be applied
            // and while doing so gets multiple updates.

            if (other.hasId)
            {
                hasId = true;
                id = other.id;
            }
            if (other.hasChannelId)
            {
                hasChannelId = true;
                channelId = other.channelId;
            }
            if (other.hasName)
            {
                hasName = true;
                name = other.name;
            }
            if (other.hasComment)
            {
                hasComment = true;
                comment = other.comment;
            }
            if (other.hasHash)
            {
                hasHash = true;
                hash = other.hasHash;
            }
            if (other.hasSelfMute)
            {
                hasSelfMute = true;
                selfMuted = other.selfMuted;
            }
            if (other.hasSelfDeaf)
            {
                hasSelfDeaf = true;
                selfDeaf = other.selfDeaf;
            }
        }

        // State change indicators
        bool hasId, hasChannelId, hasName, hasComment, hasHash, hasSelfMute, hasSelfDeaf;

        // Actual state value if indicator for it is true
        uint id, channelId;
        QString name, comment, hash;
        bool selfMuted, selfDeaf, isMe;
    };

    struct VoicePacketInfo
    {
        VoicePacketInfo(std::vector<QByteArray> encodedFrames_) 
        {
            isLoopBack = false;
            isPositional = false;
            encodedFrames = encodedFrames_;
            pos = float3::zero;
        }

        bool isLoopBack;
        bool isPositional;
        float3 pos;

        std::vector<QByteArray> encodedFrames;
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

    inline QString PermissionName(ACLPermission p) 
    {
        switch (p) 
        {
            case ACLNone:
                return "None";
            case ACLWrite:
                return "Write ACL";
            case ACLTraverse:
                return "Traverse";
            case ACLEnter:
                return "Enter";
            case ACLSpeak:
                return "Speak";
            case ACLWhisper:
                return "Whisper";
            case ACLMuteDeafen:
                return "Mute/Deafen";
            case ACLMove:
                return "Move";
            case ACLMakeChannel:
                return "Make channel";
            case ACLMakeTempChannel:
                return "Make temporary";
            case ACLLinkChannel:
                return "Link channel";
            case ACLTextMessage:
                return "Text message";
            case ACLKick:
                return "Kick";
            case ACLBan:
                return "Ban";
            case ACLRegister:
                return "Register User";
            case ACLSelfRegister:
                return "Register Self";
            default:
                break;
        }
        return QString();
    }
}
/// @endcond
