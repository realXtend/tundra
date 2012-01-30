// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "FrameworkFwd.h"
#include "MumbleFwd.h"
#include "MumbleDefines.h"

#include "SoundBuffer.h"
#include "SoundChannel.h"

#include "speex/speex_preprocess.h"

#include <QThread>
#include <QMutex>

namespace MumbleAudio
{
    typedef std::map<uint, SoundChannelPtr> AudioChannelMap;

    class AudioProcessor : public QThread
    {
        Q_OBJECT

    public:
        AudioProcessor(Framework *framework_, MumbleAudio::AudioSettings settings);
        ~AudioProcessor();

        float levelPeakMic;
        float levelMic;

        bool isSpeech;
        bool wasPreviousSpeech;

    protected:
        // QThread override.
        void run(); 

    public slots:
        QList<QByteArray> ProcessOutputAudio();
        void SetOutputAudioMuted(bool outputAudioMuted_);

        void PlayInputAudio(QList<uint> mutedUserIds);
        void SetInputAudioMuted(bool inputAudioMuted_);
        
        void ApplySettings(AudioSettings settings);
        AudioSettings GetSettings();

        void ClearInputAudio();
        void ClearInputAudio(uint userId);
        void ClearOutputAudio();

        int CodecBitStreamVersion();

    private slots:
        void OnAudioReceived(uint userId, QList<QByteArray> frames);
        
    private:
        void ResetSpeexProcessor();
        void ClearPendingChannels();

        void PrintCeltError(int celtError, bool decoding);

        Framework *framework;
        CeltCodec *codec;

        SpeexPreprocessState *speexPreProcessor;
        AudioSettings audioSettings;
        
        AudioChannelMap userChannels;
        AudioFrameMap inputFrames;

        QList<QByteArray> pendingEncodedFrames;
        QList<QByteArray> pendingVADPreBuffer;
        QList<uint> pendingSoundChannelRemoves;

        bool outputAudioMuted;
        bool outputPreProcessed;
        bool inputAudioMuted;
        bool preProcessorReset;

        QMutex mutexCodec;
        QMutex mutexInput;
        QMutex mutexAudioMute;
        QMutex mutexAudioSettings;
        QMutex mutexAudioChannels;

        int qualityBitrate;
        int qualityFramesPerPacket;

        int holdFrames;

        QString LC;
    };
}
