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
    //////////////////////////////////////////////////////

    typedef std::deque<SoundBuffer> AudioFrameDeque;

    struct UserAudioState
    {
        UserAudioState ()
        {
            lastSeq = 0;
            isPositional = false;
            pos = float3::zero;
            frames.clear();
            soundChannel.reset();
        }

        uint lastSeq;
        bool isPositional;
        float3 pos;
        AudioFrameDeque frames;
        SoundChannelPtr soundChannel;
    };
    
    typedef std::map<uint, UserAudioState > AudioStateMap;

    //////////////////////////////////////////////////////

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
        ByteArrayList ProcessOutputAudio();
        void SetOutputAudioMuted(bool outputAudioMuted_);

        /// Plays all input audio frames from other users. 
        /// Updates MumbleUser::isPositional and emits MumbleUser::PositionalChanged
        /// and MumblePlugin::UserPositionalChanged
        void PlayInputAudio(MumblePlugin *mumble);
        void SetInputAudioMuted(bool inputAudioMuted_);
        
        void ApplySettings(AudioSettings settings);
        AudioSettings GetSettings();

        void ClearInputAudio();
        void ClearInputAudio(uint userId);
        void ClearOutputAudio();

        int CodecBitStreamVersion();

    private slots:
        void OnAudioReceived(uint userId, uint seq, ByteArrayList frames, bool isPositional, float3 pos);
        
    private:
        void ResetSpeexProcessor();
        void ClearPendingChannels();

        void PrintCeltError(int celtError, bool decoding);

        Framework *framework;
        CeltCodec *codec;

        SpeexPreprocessState *speexPreProcessor;
        AudioSettings audioSettings;

        AudioStateMap inputAudioStates;

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
