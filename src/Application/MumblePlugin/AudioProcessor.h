// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "FrameworkFwd.h"
#include "MumbleFwd.h"
#include "MumbleDefines.h"

#include "SoundBuffer.h"
#include "SoundChannel.h"

#include <QThread>
#include <QMutex>

namespace MumbleAudio
{
    typedef std::map<uint, SoundChannelPtr> AudioChannelMap;

    class AudioProcessor : public QThread
    {
        Q_OBJECT

    public:
        AudioProcessor(Framework *framework_);
        ~AudioProcessor();

    protected:
        // QThread override.
        void run(); 

    public slots:
        QList<QByteArray> ProcessOutputAudio();

        void PlayInputAudio(QList<uint> mutedUserIds);
        
        void ClearInputAudio();
        void ClearInputAudio(uint userId);
        void ClearOutputAudio();

        void SetOutputAudioMuted(bool outputAudioMuted_);
        void SetInputAudioMuted(bool inputAudioMuted_);
        void SetOutputQuality(MumbleAudio::AudioQuality quality);
        void SetFramesPerPacket(int frames);

        int CodecBitStreamVersion();

    private slots:
        void OnAudioReceived(uint userId, QList<QByteArray> frames);
        
    private:
        void PrintCeltError(int celtError, bool decoding);
        void ClearPendingChannels();

        Framework *framework;
        CeltCodec *codec;
        
        AudioChannelMap userChannels;
        AudioFrameMap inputFrames;

        QList<QByteArray> outFramesPending;
        QList<uint> pendingChannelRemoves;

        bool outputAudioMuted;
        bool inputAudioMuted;

        QMutex mutexCodec;
        QMutex mutexInput;
        QMutex mutexAudioMute;
        QMutex mutexAudioQuality;
        QMutex mutexAudioChannels;

        QString LC;

        int qualityBitrate;
        int qualityFramesPerPacket;

        AudioQuality outputQuality;
    };
}
