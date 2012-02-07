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

        void GetLevels(float &peakMic, bool &speaking);

    protected:
        // QThread override.
        void run(); 

        // QObject override.
        void timerEvent(QTimerEvent *event);

    public slots:
        ByteArrayVector ProcessOutputAudio();
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

        // Do not call this after the thread starts. Get the bitstream version before that.
        // Calling it is safe but -1 will be returned.
        int CodecBitStreamVersion();

    private slots:
        void OnAudioReceived(uint userId, uint seq, ByteArrayVector frames, bool isPositional, float3 pos);
        
    private:
        void ResetSpeexProcessor();
        void ClearPendingChannels();

        void PrintCeltError(int celtError, bool decoding);

        // Below floats and booleans need mutexOutputPCM lock for reading. Use the GetLevels function.
        float levelPeakMic;
        float levelMic;
        bool isSpeech;
        bool wasPreviousSpeech;

        // Used only in the main thread.
        Framework *framework;

        // Used in audio thread without locks.
        CeltCodec *codec;

        // Used in audio thread without locks.
        SpeexPreprocessState *speexPreProcessor;
        
        // Used in both main and audio thread with mutexAudioSettings. 
        AudioSettings audioSettings;

        // Used in both main and audio thread with mutexInput.
        AudioStateMap inputAudioStates;

        // Used in both main and audio thread with mutexOutputEncoded.
        QList<QByteArray> pendingEncodedFrames;
        
        // Used in both main and audio thread with mutexAudioChannels.
        // SoundChannels shared ptrs cannot be reseted in the audio thread
        // so they are marked down for removal into this list with userId.
        QList<uint> pendingSoundChannelRemoves;

        // Used in both main and audio thread with mutexOutputPCM.
        std::vector<SoundBuffer> pendingPCMFrames;

        // Used in audio thread without locks.
        QList<QByteArray> pendingVADPreBuffer;

        // Used in both main and audio thread with mutexAudioMute.
        bool outputAudioMuted;

        // Used in both main and audio thread with mutexAudioMute.
        bool inputAudioMuted;

        // Used in both main and audio thread with mutexAudioSettings.
        bool outputPreProcessed;

        // Used in main thread without locks.
        bool preProcessorReset;

        // Various mutexes for sharing data between audio and main thread.
        QMutex mutexInput;
        QMutex mutexOutputPCM;
        QMutex mutexOutputEncoded;
        QMutex mutexAudioMute;
        QMutex mutexAudioSettings;
        QMutex mutexAudioChannels;

        int qualityBitrate;
        int qualityFramesPerPacket;

        int holdFrames;
        int qobjTimerId_;

        QString LC;
    };
}
