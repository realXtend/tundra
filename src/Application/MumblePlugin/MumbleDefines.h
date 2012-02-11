// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "MumbleNetwork.h"

#include <QObject>
#include <QByteArray>
#include <QString>

#include <vector>
#include <deque>
#include <map>

typedef std::vector<QByteArray> ByteArrayVector;

struct MumblePluginState
{
    bool serverSynced;
    uint sessionId;
    MumbleNetwork::ConnectionState connectionState;
    MumbleNetwork::NetworkMode networkMode;
    QString username;
    QString address;
    ushort port;

    bool outputAudioMuted;
    bool outputAudioLoopBack;
    bool outputPositional;
    bool inputAudioMuted;

    QString fullChannelName;

    MumblePluginState()
    {
        Reset();

        // Certain values are only reset on module startup
        // and kept to set values between multiple connections.
        outputPositional = false;
    }

    void Reset()
    {
        serverSynced = false;
        sessionId = 0;
        connectionState = MumbleNetwork::MumbleDisconnected;
        networkMode = MumbleNetwork::MumbleTCPMode;
        username = "";
        address = "";
        port = 0;
        outputAudioMuted = true;
        inputAudioMuted = true;
        outputAudioLoopBack = false;
        fullChannelName = "";
    }

    QString FullHost() 
    { 
        if (address.isEmpty() || port == 0) 
            return ""; 
        else
            return QString("%1:%2").arg(address).arg(port); 
    }

    QString PortToString() 
    { 
        if (port == 0) 
            return ""; 
        else 
            return QString::number(port); 
    }
};

namespace MumbleAudio
{
    enum AudioQuality
    {
        QualityNotSet,
        QualityLow,
        QualityBalanced,
        QualityHigh
    };

    enum TransmitMode
    {
        TransmitContinuous,
        TransmitVoiceActivity
    };

    class AudioSettings : public QObject
    {
    Q_OBJECT

    public:
        AudioSettings()
        {
            // Default settings
            quality = QualityBalanced;
            transmitMode = TransmitContinuous;
            suppression = -30;
            amplification = 19000;
            VADmin = 0.80f;
            VADmax = 0.98f;
            innerRange = 30;
            outerRange = 75;
            allowSendingPositional = true;
            allowReceivingPositional = true;
        }

        AudioSettings(const MumbleAudio::AudioSettings &other)
        {
            quality = other.quality;
            transmitMode = other.transmitMode;
            suppression = other.suppression;
            amplification = other.amplification;
            VADmin = other.VADmin;
            VADmax = other.VADmax;
            innerRange = other.innerRange;
            outerRange = other.outerRange;
            allowSendingPositional = other.allowSendingPositional;
            allowReceivingPositional = other.allowReceivingPositional;
        }

        MumbleAudio::AudioSettings &operator=(const MumbleAudio::AudioSettings &other)
        {
            quality = other.quality;
            transmitMode = other.transmitMode;
            suppression = other.suppression;
            amplification = other.amplification;
            VADmin = other.VADmin;
            VADmax = other.VADmax;
            innerRange = other.innerRange;
            outerRange = other.outerRange;
            allowSendingPositional = other.allowSendingPositional;
            allowReceivingPositional = other.allowReceivingPositional;
            return *this;
        }

        AudioQuality quality;
        TransmitMode transmitMode;
        int suppression;
        int amplification;
        float VADmin;
        float VADmax;
        int innerRange;
        int outerRange;
        bool allowSendingPositional;
        bool allowReceivingPositional;
    };

    static int MUMBLE_AUDIO_SAMPLE_RATE = 48000;
    static int MUMBLE_AUDIO_SAMPLE_WIDTH = 16;
    static int MUMBLE_AUDIO_SAMPLES_IN_FRAME = 480;

    static int MUMBLE_AUDIO_QUALITY_LOW = 16000;
    static int MUMBLE_AUDIO_QUALITY_BALANCED = 40000;
    static int MUMBLE_AUDIO_QUALITY_ULTRA = 72000;
    static int MUMBLE_AUDIO_FRAMES_PER_PACKET_LOW = 5; // mumble original 6
    static int MUMBLE_AUDIO_FRAMES_PER_PACKET_BALANCED = 5; // mumble original 2
    static int MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA = 5; // mumble original 1
}

#ifdef Q_OS_WIN
#define MUMBLE_STACKVAR(type, varname, count) type *varname=reinterpret_cast<type *>(_alloca(sizeof(type) * (count)))
#else
#define MUMBLE_STACKVAR(type, varname, count) type varname[count]
#endif
