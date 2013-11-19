// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "FrameworkFwd.h"
#include "MumbleFwd.h"
#include "MumbleDefines.h"

#include "SoundBuffer.h"
#include "SoundChannel.h"

#include <speex/speex_preprocess.h>
#include <speex/speex_echo.h>

#include <QThread>
#include <QMutex>
#include <QReadWriteLock>
#include <QTimer>

#ifdef Q_WS_MAC
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

/// @cond PRIVATE
namespace MumbleAudio
{
    //////////////////////////////////////////////////////

    typedef std::deque<SoundBuffer> AudioFrameDeque;

    struct UserAudioState
    {
        explicit UserAudioState();
        ~UserAudioState();

        uint lastSeq;
        bool isPositional;
        float3 pos;
        AudioFrameDeque frames;
        AudioFrameDeque playedFrames;
        SoundChannelPtr soundChannel;
        CeltCodec *codec;
    };

    typedef std::map<uint, UserAudioState* > AudioStateMap;

    struct UserOutputAudioState
    {
        explicit UserOutputAudioState();

        bool isLoopBack;
        bool isPositional;
        float3 position;
        size_t numberOfFrames;
    };

    class AudioRecorder : public QObject
    {
        Q_OBJECT
    public:
        AudioRecorder();
        ~AudioRecorder();

        bool StartRecording(const QString &name, uint frequency, bool is16bit, bool stereo, uint bufferSize);
        void StopRecording();
        uint RecordedSoundSize() const;
        uint RecordedSoundData(void *data, uint size);
    private:
        uint captureSampleSize_;
        ALCdevice *captureDevice_;
    };

    //////////////////////////////////////////////////////

    class AudioProcessor : public QThread
    {
        Q_OBJECT

    public:
        AudioProcessor(Framework *framework_, MumbleAudio::AudioSettings settings, MumbleNetworkHandler *networkHandler);
        ~AudioProcessor();

        void GetLevels(float &peakMic, bool &speaking);

    protected:
        // QThread override.
        void run();

        // QObject override.
        void timerEvent(QTimerEvent *event);

    public slots:
        void ProcessOutputAudio();
        void SetOutputAudioMuted(bool outputAudioMuted_);

        void SetUserOutputState(UserOutputAudioState state);
        UserOutputAudioState UserOutputState();

        /// Plays all input audio frames from other users.
        /// Updates MumbleUser::isPositional and emits MumbleUser::PositionalChanged
        /// and MumblePlugin::UserPositionalChanged
        void PlayInputAudio(MumblePlugin *mumble);
        void SetInputAudioMuted(bool inputAudioMuted_);
        SoundChannelPtr CreateVoiceChannel(const SoundBuffer &buffer);
        AudioAssetPtr CreateAudioAssetFromSoundBuffer(const SoundBuffer &buffer);

        void ApplyFramesPerPacket(int framesPerPacket);
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
        void OnResetFramesPerPacket();

    private:
        void ResetSpeexProcessor();
        void ClearPendingChannels();

        // Caller must hold mutexOutputPCM lock
        void DoEchoCancellation(SoundBuffer &frame);

        void PrintCeltError(int celtError, bool decoding);

        // Below floats and booleans need mutexOutputPCM lock for reading. Use the GetLevels function.
        float levelPeakMic;
        float levelMic;
        bool isSpeech;
        bool wasPreviousSpeech;

        MumbleNetworkHandler *networkHandler_;
        AudioRecorder *recorder_;

        // Used only in the main thread.
        Framework *framework;

        // Used in audio thread without locks.
        CeltCodec *codec;

        // Used in audio thread without locks.
        SpeexPreprocessState *speexPreProcessor;

        // Used in audio thread for echo cancellation
        SpeexEchoState *speexEcho;

        // Used in both main and audio thread with mutexAudioSettings.
        AudioSettings audioSettings;

        // Used in both main and audio thread with mutexInput.
        AudioStateMap inputAudioStates;

        // This user's output state
        UserOutputAudioState ownAudioState;

        // Used in both main and audio thread with mutexOutputEncoded.
        QList<QByteArray> pendingEncodedFrames;

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
        QMutex mutexRecorder;

        QReadWriteLock mutexAudioMute;
        QReadWriteLock mutexAudioSettings;
        QReadWriteLock mutexUserOutputAudioState;

        int qualityBitrate;
        int qualityFramesPerPacket;

        int bufferFullFrames;
        int holdFrames;
        int qobjTimerId;

        QTimer resetFramesPerPacket;

        QString LC;

    };
}
/// @endcond
