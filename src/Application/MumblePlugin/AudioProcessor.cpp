// For conditions of distribution and use, see copyright notice in LICENSE

#include "AudioProcessor.h"
#include "MumblePlugin.h"
#include "MumbleData.h"
#include "CeltCodec.h"
#include "celt/celt.h"

#include "Framework.h"
#include "Profiler.h"
#include "AudioAPI.h"
#include "CoreDefines.h"
#include "CoreTypes.h"
#include "LoggingFunctions.h"

#include <QMutexLocker>

namespace MumbleAudio
{
    AudioProcessor::AudioProcessor(Framework *framework_, MumbleAudio::AudioSettings settings) :
        LC("[MumbleAudioProcessor]: "),
        framework(framework_),
        codec(new CeltCodec()),
        speexPreProcessor(0),
        speexEcho(0),
        outputPreProcessed(false),
        preProcessorReset(true),
        isSpeech(false),
        wasPreviousSpeech(false),
        holdFrames(0),
        bufferFullFrames(0),
        qualityFramesPerPacket(MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA)
    {
        ApplySettings(settings);

        // Timer is created in the main thread context as it can only be started in the thread its created in,
        // and we want to start it in ProcessOutputAudio().
        resetFramesPerPacket.setSingleShot(true);
        connect(&resetFramesPerPacket, SIGNAL(timeout()), this, SLOT(OnResetFramesPerPacket()), Qt::QueuedConnection);
    }

    AudioProcessor::~AudioProcessor()
    {
        // Cleanup is done in run() when thread exits.
    }

    void AudioProcessor::ResetSpeexProcessor()
    {
        if (!preProcessorReset)
            return;
        preProcessorReset = false;

        AudioSettings currentSettings;

        mutexAudioSettings.lockForRead();
        currentSettings = audioSettings;
        mutexAudioSettings.unlock();

        outputPreProcessed = false;
        if (currentSettings.suppression < 0 || currentSettings.amplification > 0)
            outputPreProcessed = true;

        // Only usage of speexPreProcessor ptr in another thread is behind this lock.
        QMutexLocker outputLock(&mutexOutputPCM);

        if (speexPreProcessor)
        {
            speex_preprocess_state_destroy(speexPreProcessor);
            speexPreProcessor = 0;
        }

        speexPreProcessor = speex_preprocess_state_init(MUMBLE_AUDIO_SAMPLES_IN_FRAME, MUMBLE_AUDIO_SAMPLE_RATE);

        int enabled = 1;
        int arg = 0;
        float fArg = 0.0f;

        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_VAD, &enabled);
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC, &enabled);
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DENOISE, &enabled);
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DEREVERB, &enabled);

        arg = 30000;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_TARGET, &arg);

        float v = 30000.0f / static_cast<float>(currentSettings.amplification);
        arg = static_cast<int>(floorf(20.0f * log10f(v)));
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &arg);

        arg = 12;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_INCREMENT, &arg);

        arg = -60;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_DECREMENT, &arg);

        arg = currentSettings.suppression;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &arg);

        fArg = 0.0f;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &fArg);
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &fArg);

        if (speexEcho) 
        {
            speex_echo_state_destroy(speexEcho);
            speexEcho = 0;
        }

        // Enable echo cancellation if its enabled in the settings.
        if (currentSettings.echoCancellation)
        {
            // Initialize echo cancellation state, using the same frame size as used for sending
            // and receiving (should be 10-20 ms), and filter length of 250 ms (quarter of a second)
            speexEcho = speex_echo_state_init(MUMBLE_AUDIO_SAMPLES_IN_FRAME, MUMBLE_AUDIO_SAMPLE_RATE / 4);

            // Use fixed sample rate
            arg = MUMBLE_AUDIO_SAMPLE_RATE;
            speex_echo_ctl(speexEcho, SPEEX_ECHO_SET_SAMPLING_RATE, &arg);
            speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_ECHO_STATE, speexEcho);
        }
        else
            speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_ECHO_STATE, 0);
    }

    void AudioProcessor::run()
    {
        qobjTimerId = startTimer(15); // Audio processing with ~60 fps.

        exec(); // Blocks untill quit()

        killTimer(qobjTimerId);

        SAFE_DELETE(codec);

        ClearInputAudio();
        framework = 0;

        if (speexPreProcessor)
        {
            speex_preprocess_state_destroy(speexPreProcessor);
            speexPreProcessor = 0;
        }
        if (speexEcho) 
        {
            speex_echo_state_destroy(speexEcho);
            speexEcho = 0;
        }
    }

    void AudioProcessor::timerEvent(QTimerEvent *event)
    {
        if (event->timerId() != qobjTimerId)
            return;

        // This function processed queued PCM frames with speexdsp and celt at ~60fps and adds them to
        // a pending encoded frames list to be sent out to the network from the main thread.
        // Mutex mutexOutputPCM and mutexOutputEncoded are the main locks for queuing the frames back and forth.
        if (!codec)
            return;

        int localGain = 0;

        mutexAudioSettings.lockForRead();
        int localQualityBitrate = qualityBitrate;
        int localSuppress = audioSettings.suppression;
        bool detectVAD = audioSettings.transmitMode == TransmitVoiceActivity;
        bool localPreProcess = outputPreProcessed;
        float VADmin = audioSettings.VADmin;
        float VADmax = audioSettings.VADmax;
        mutexAudioSettings.unlock();

        mutexOutputPCM.lock();
        if (pendingPCMFrames.size() == 0)
        {
            mutexOutputPCM.unlock();
            return;
        }

        QList<QByteArray> encodedFrames;
        for (std::vector<SoundBuffer>::iterator pcmIter = pendingPCMFrames.begin(); pcmIter != pendingPCMFrames.end(); ++pcmIter)
        {
            SoundBuffer &pcmFrame = (*pcmIter);
            if (pcmFrame.data.size() == 0)
                continue;

            isSpeech = true;
            DoEchoCancellation(pcmFrame);

            if (localPreProcess)
            {
                speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_GET_AGC_GAIN, &localGain);
                int suppression = localSuppress - localGain;
                if (suppression > 0)
                    suppression = 0;
                speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &suppression);
                speex_preprocess_run(speexPreProcessor, (spx_int16_t*)&pcmFrame.data[0]);

                if (detectVAD)
                {
                    float sum = 1.0f;
                    short *data = (short*)&pcmFrame.data[0];
                    for (int index=0; index<MUMBLE_AUDIO_SAMPLES_IN_FRAME; index++)
                    {
                        int value = data[index];
                        sum += static_cast<float>(value * value);
                    }

                    levelPeakMic = qMax(20.0f * log10f(sqrtf(sum / static_cast<float>(MUMBLE_AUDIO_SAMPLES_IN_FRAME)) / 32768.0f), -96.0f);
                    levelPeakMic = qMax(levelPeakMic - localGain, -96.0f);
                    levelMic = (1.0f + levelPeakMic / 96.0f);

                    // Detect mic level if speaking
                    if (levelMic > VADmax)
                        isSpeech = true;
                    else if (levelMic > VADmin && wasPreviousSpeech)
                        isSpeech = true;
                    else
                        isSpeech = false;

                    if (isSpeech)
                        holdFrames = 0;
                    else
                    {
                        // Hold certain amount of frames even if not speaking.
                        // This allows end of sentences to get to the outgoing buffer safely.
                        holdFrames++;
                        if (holdFrames < 20)
                            isSpeech = true;
                    }
                }
            }

            // Encode
            unsigned char compressedBuffer[512];
            int bytesWritten = codec->Encode(pcmFrame, compressedBuffer, localQualityBitrate);
            if (bytesWritten > 0)
            {
                QByteArray encodedFrame(reinterpret_cast<const char*>(compressedBuffer), bytesWritten);

                // If speech, add to encoded frames. But first
                // append any 'prediction' buffered frames so start of sentences
                // can get to the outgoing buffer safely.
                if (isSpeech || wasPreviousSpeech)
                {
                    if (detectVAD && pendingVADPreBuffer.size() > 0)
                    {
                        encodedFrames.append(pendingVADPreBuffer);
                        pendingVADPreBuffer.clear();
                    }
                    encodedFrames.push_back(encodedFrame);
                }
                // If voice activity detection is enabled but this is
                // not speech, add the frame to the VAD 'prediction' buffer.
                else if (detectVAD && !isSpeech && !wasPreviousSpeech)
                {
                    while(pendingVADPreBuffer.size() >= 5)
                    {
                        if (!pendingVADPreBuffer.isEmpty())
                            pendingVADPreBuffer.removeFirst();
                        else
                            break;
                    }
                    pendingVADPreBuffer.push_back(encodedFrame);
                }
            }
            wasPreviousSpeech = isSpeech;
        }
        pendingPCMFrames.clear();
        mutexOutputPCM.unlock();

        // Push encoded frames for the main thread to send out to network.
        mutexOutputEncoded.lock();
        for(int i=0; i<encodedFrames.size(); ++i)
            pendingEncodedFrames.append(QByteArray(encodedFrames.at(i)));
        mutexOutputEncoded.unlock();
    }

    void AudioProcessor::GetLevels(float &peakMic, bool &speaking)
    {
        // The peak mic level and is speaking are written in a mutexOutputPCM lock.
        // So use the same lock to read the data out for main thread usage.
        if (mutexOutputPCM.tryLock(15))
        {
            peakMic = levelPeakMic;
            speaking = isSpeech;
            mutexOutputPCM.unlock();
        }
        else
        {
            peakMic = 0.f;
            speaking = false;
        }
    }

    void AudioProcessor::DoEchoCancellation(SoundBuffer &pcmFrame)
    {
        // If not enabled this is null.
        if (!speexEcho)
            return;

        SoundBuffer outBuf;
        SoundBuffer playedFrame;
        u16 *playedFrameData;

        playedFrame.data.resize(MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8, 0);
        playedFrameData = reinterpret_cast<u16*>(&playedFrame.data[0]);

        if (mutexInput.tryLock(15))
        {
            for (AudioStateMap::iterator iter = inputAudioStates.begin(); iter != inputAudioStates.end(); ++iter)
            {
                UserAudioState *userAudioState = iter->second;
                if (!userAudioState || userAudioState->playedFrames.empty())
                    continue;

                const SoundBuffer &buf = userAudioState->playedFrames.front();
                if (buf.data.size() == playedFrame.data.size())
                {
                    const u16 *bufData = reinterpret_cast<const u16*>(&buf.data[0]);
                    for (size_t i = 0; i < buf.data.size() / 2; i++)
                        playedFrameData[i] += bufData[i];
                }
                userAudioState->playedFrames.pop_front();
            }
            mutexInput.unlock();
        }
        else
            return;

        outBuf.data.resize(pcmFrame.data.size());

        // Input (mic) data is from pcmFrame. Output (speaker) data mixed from 
        // separate input streams. Echo-cancelled data is put to outBuf.
        speex_echo_cancellation(speexEcho, (spx_int16_t*)&pcmFrame.data[0], (spx_int16_t*)&playedFrame.data[0], (spx_int16_t*)&outBuf.data[0]);

        // Copy echo-cancelled output over existing output PCM frames.
        pcmFrame.data.swap(outBuf.data);
    }

    void AudioProcessor::SetOutputAudioMuted(bool outputAudioMuted_)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        mutexAudioMute.lockForWrite();
        outputAudioMuted = outputAudioMuted_;
        mutexAudioMute.unlock();

        if (!outputAudioMuted_)
        {
            mutexAudioSettings.lockForWrite();

            // Reset back to ultra state, gets increased automatically if necessary.
            qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA;

            // Reset processor. Clears echo cancellation state.
            ResetSpeexProcessor();

            // Recording buffer of 40 celt frames, 38400 bytes (overcompensate a bit)
            int bufferSize = (MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8) * 40;
            if (!framework->Audio()->StartRecording(audioSettings.recordingDevice, MUMBLE_AUDIO_SAMPLE_RATE, true, false, bufferSize))
            {
                LogWarning("Could not open recording device '" + audioSettings.recordingDevice + "'. Trying to open the default device instead.");
                framework->Audio()->StartRecording("", MUMBLE_AUDIO_SAMPLE_RATE, true, false, bufferSize);
            }

            mutexAudioSettings.unlock();
        }
        else
        {
            framework->Audio()->StopRecording();

            // Clear obsolete playback history for echo cancellation.
            if (mutexInput.tryLock(15))
            {
                for (AudioStateMap::iterator iter = inputAudioStates.begin(); iter != inputAudioStates.end(); ++iter)
                    if (iter->second)
                        iter->second->playedFrames.clear();
                mutexInput.unlock();
            }
        }

        ClearOutputAudio();
    }

    void AudioProcessor::SetInputAudioMuted(bool inputAudioMuted_)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        mutexAudioMute.lockForWrite();
        inputAudioMuted = inputAudioMuted_;
        mutexAudioMute.unlock();

        ClearInputAudio();
    }

    void AudioProcessor::ApplyFramesPerPacket(int framesPerPacket)
    {
        if (framesPerPacket < 2)
            framesPerPacket = 2;
        if (framesPerPacket > 10)
            framesPerPacket = 10;

        LogWarning(LC + "Changing frames per packet to " + QString::number(framesPerPacket) + ", changing the rate is not recommended for other than debugging!");

        mutexAudioSettings.lockForWrite();
        bufferFullFrames = 0;
        qualityFramesPerPacket = framesPerPacket;
        mutexAudioSettings.unlock();

        ClearOutputAudio();
    }

    void AudioProcessor::ApplySettings(AudioSettings settings)
    {
        bool positionalRangesChanged = false;
        bool recondingDeviceChanged = false;
        int changedInnerRange = 0;
        int changedOuterRange = 0;

        // This function is called in the main thread
        mutexAudioSettings.lockForWrite();

        // Detect recording device changed
        if (audioSettings.recordingDevice != settings.recordingDevice)
            recondingDeviceChanged = true;

        // Detect if positional playback ranges changed.
        if (audioSettings.innerRange != settings.innerRange || audioSettings.outerRange != settings.outerRange)
        {
            positionalRangesChanged = true;
            changedInnerRange = settings.innerRange;
            changedOuterRange = settings.outerRange;
        }

        audioSettings = settings;
        if (audioSettings.suppression > 0)
            audioSettings.suppression = 0;

        switch(audioSettings.quality)
        {
            case QualityLow:
            {
                qualityBitrate = MUMBLE_AUDIO_QUALITY_LOW;
                break;
            }
            case QualityBalanced:
            {
                qualityBitrate = MUMBLE_AUDIO_QUALITY_BALANCED;
                break;
            }
            case QualityHigh:
            {
                qualityBitrate = MUMBLE_AUDIO_QUALITY_ULTRA;
                break;
            }
        }

        // Reset back to ultra state, gets increased automatically if necessary.
        bufferFullFrames = 0;
        qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA;

        mutexAudioSettings.unlock();

        // Apply new positional ranges to existing positional sound channels.
        if (positionalRangesChanged)
        {
            if (mutexInput.tryLock(15))
            {
                for (AudioStateMap::iterator iter = inputAudioStates.begin(); iter != inputAudioStates.end(); ++iter)
                {
                    UserAudioState *userAudioState = iter->second;
                    if (userAudioState->soundChannel.get() && userAudioState->soundChannel->IsPositional())
                        userAudioState->soundChannel->SetRange(static_cast<float>(changedInnerRange), static_cast<float>(changedOuterRange), 1.0f);
                }
                mutexInput.unlock();
            }
        }

        preProcessorReset = true;
        ResetSpeexProcessor();

        // Start recording with the new device if changed.
        // If recording is not enabled, change to false,
        // it will be applied on the when output mute state is changed.
        if (recondingDeviceChanged)
        {
            mutexAudioMute.lockForRead();
            if (outputAudioMuted)
                recondingDeviceChanged = false;
            mutexAudioMute.unlock();
        }

        if (recondingDeviceChanged)
        {
            LogInfo(LC + "Recording device change detected to '" + (settings.recordingDevice.isEmpty() ? "Default Recording Device" : settings.recordingDevice) + "'.");
            framework->Audio()->StopRecording();

            // Recording buffer of 40 celt frames, 38400 bytes (overcompensate a bit)
            int bufferSize = (MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8) * 40;
            framework->Audio()->StartRecording(settings.recordingDevice, MUMBLE_AUDIO_SAMPLE_RATE, true, false, bufferSize);

            ClearOutputAudio();
        }
    }

    MumbleAudio::AudioSettings AudioProcessor::GetSettings()
    {
        mutexAudioSettings.lockForRead();
        MumbleAudio::AudioSettings threadSettings = audioSettings;
        mutexAudioSettings.unlock();
        return threadSettings;
    }

    ByteArrayVector AudioProcessor::ProcessOutputAudio()
    {
        // This function is called in the main thread
        if (!framework)
            return ByteArrayVector();

        // Get recorded PCM frames from AudioAPI.
        PROFILE(Mumble_ProcessOutputAudio_Queue_Encoding)
        std::vector<SoundBuffer> pcmFrames;
        uint celtFrameSize = MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8;
        while (framework->Audio()->GetRecordedSoundSize() >= celtFrameSize)
        {
            SoundBuffer outputPCM;
            outputPCM.data.resize(celtFrameSize);
            uint bytesOut = framework->Audio()->GetRecordedSoundData(&outputPCM.data[0], celtFrameSize);
            if (bytesOut == celtFrameSize)
                pcmFrames.push_back(outputPCM);
        }
        if (pcmFrames.size() > 0)
        {
            // We want to fail here if something is about to give on the mutexes.
            if (mutexOutputPCM.tryLock(15))
            {
                pendingPCMFrames.insert(pendingPCMFrames.end(), pcmFrames.begin(), pcmFrames.end());
                mutexOutputPCM.unlock();
            }
            else
                LogDebug(LC + QString("Failed to acquire pending PCM frames mutex for %1 frames").arg(pcmFrames.size()));
        }
        ELIFORP(Mumble_ProcessOutputAudio_Queue_Encoding)

        PROFILE(Mumble_ProcessOutputAudio_Get_Encoded)
        QMutexLocker lockEncoded(&mutexOutputEncoded);

        // No queued encoded frames for network.
        if (pendingEncodedFrames.size() == 0)
            return ByteArrayVector();

        // Get packet count per frame.
        mutexAudioSettings.lockForRead();
        int framesPerPacket = qualityFramesPerPacket;
        mutexAudioSettings.unlock();

         /** Ensure we are not buffing faster than what is sent to network. Increasing the frames per packet (automatically) is not a good thing.
            This happens when our main thread gets blocked and our encoded frames gets filled.
            We can increase the frames per packet to catch up quickly, but going anything above 2-4 is bad found sound quality. Hence we reset back to
            ULTRA == 2 after 5 seconds, we assume the mainloop blockage like loading heavy assets has ended and want to return to a proper voice network
            sync rate. If your FPS is bad all the time, eg. in a particular heavy scene to render, this will keep hitting and there is little we can do, if you
            have constant < 25 FPS in Tundra your voip will quality will suck anyways.

            This is Tundra VOIPs main problem: We are trying to stream and receive real time voice audio that will at times get interrupted by something
            blocking the main thread. This is why we have one audio thread and secondary network thread and process voip at 60 fps if we can. The upside
            of normal voip clients like Mumble is that they don't have 3D rendering interrupting them and they can focus on doing voice stuff ;)

            @todo Remove OpenAL usage for input microphone and 3D positional playback. Thread microphone by using WASAPI on windows and something on linux/mac.
            Research our options for threaded recording/playback without using Framework or AudioAPI pointers in this audio processing thread.
        */
        if (pendingEncodedFrames.size() > framesPerPacket * 10)
        {
            // Do some helpful info logs if we are auto increasing frames per packet count.
            if (framesPerPacket > 8)
                LogInfo(LC + QString("Output buffer full with %1/%2 frames, frames/packet is %3").arg(pendingEncodedFrames.size()).arg(framesPerPacket*10).arg(framesPerPacket));

            // Remove oldest frames to get the buffer to a acceptable size.
            while(pendingEncodedFrames.size() > framesPerPacket * 10)
            {
                if (!pendingEncodedFrames.isEmpty())
                    pendingEncodedFrames.removeFirst();
                else
                    break;
            }

            mutexAudioSettings.lockForWrite();
            bufferFullFrames++;
            if (bufferFullFrames >= 5 && qualityFramesPerPacket <= 8)
            {
                LogInfo(LC + QString("Output buffer full with %1/%2 frames, auto increasing frames/packet to %3 due to potential main thread blockage.").arg(pendingEncodedFrames.size()).arg(framesPerPacket*10).arg(framesPerPacket+2));

                bufferFullFrames = 0;
                qualityFramesPerPacket += 2;
                framesPerPacket = qualityFramesPerPacket;

                if (!resetFramesPerPacket.isActive())
                    resetFramesPerPacket.start(5000);
            }
            mutexAudioSettings.unlock();
        }

        // If we are speaking send out full 'framesPerPacket' frames. If we are not speaking send whatever is left in the buffer but max is still 'framesPerPacket'.
        int framesToPacket = (isSpeech || wasPreviousSpeech) ? framesPerPacket : qMin(framesPerPacket, pendingEncodedFrames.size());

        // Enough encoded frames in the ready queue
        if (pendingEncodedFrames.size() >= framesToPacket)
        {
            ByteArrayVector sendOutNow;
            for (int i=0; i<framesToPacket; ++i)
            {
                if (!pendingEncodedFrames.isEmpty())
                    sendOutNow.push_back(pendingEncodedFrames.takeFirst());
                else
                    break;
            }
            return sendOutNow;
        }
        else
            return ByteArrayVector();
    }

    void AudioProcessor::PlayInputAudio(MumblePlugin *mumble)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        // Read playback settings
        mutexAudioSettings.lockForRead();
        int allowReceivingPositional = audioSettings.allowReceivingPositional;
        int positionalInnerRange = allowReceivingPositional ? audioSettings.innerRange : 0;
        int positionalOuterRange = allowReceivingPositional ? audioSettings.outerRange : 0;
        mutexAudioSettings.unlock();

        mutexAudioMute.lockForRead();
        bool localOutputAudioMuted = outputAudioMuted;
        mutexAudioMute.unlock();

        // Lock user audio states. This is the place we want to fail on a tryLock as we can play the frames later as well.
        // If this fails too many times we just remove the oldest frames.
        if (!mutexInput.tryLock(15))
        {
            LogDebug(LC + "PlayInputAudio tryLock(15) failed to acquire lock!");
            return;
        }
        if (inputAudioStates.empty())
        {
            mutexInput.unlock();
            return;
        }

        AudioStateMap::iterator end = inputAudioStates.end();
        for (AudioStateMap::iterator iter = inputAudioStates.begin(); iter != end; ++iter)
        {
            uint userId = iter->first;
            UserAudioState *userAudioState = iter->second;
            if (userAudioState == 0)
                continue;

            // We must have the user if we are receiving audio from him.
            MumbleUser *user = mumble->User(userId);
            if (!user)
                continue;

            // When muted don't play any pending frames, just delete them.
            if (user->isMuted)
            {
                userAudioState->frames.clear();
                userAudioState->playedFrames.clear();
            }

            // Check speaking state, not speaking if pending frames is empty and SoundChannel is not playing.
            if (user->isMuted || userAudioState->frames.empty())
            {
                bool playing = false;
                if (userAudioState->soundChannel.get())
                {
                    // Continue showing "playing" state until all queued buffers have been played.
                    // Remove the sound channel once we are done with it 1) muted 2) no pending frames, user is silent.
                    if (userAudioState->soundChannel->State() == SoundChannel::Playing)
                        playing = true;
                }
                if (!playing)
                    user->SetAndEmitSpeaking(false);
                continue;
            }

            // Report if we are getting too much buffers from a single user. We don't want to feed OpenAL too many
            // buffers as it can crash and also there is not sense in playing very old audio.
            if (userAudioState->frames.size() > 150)
            {
                LogWarning(LC + QString("Input frame buffer size too high (%1) for user id %2, removing oldest.").arg(userAudioState->frames.size()).arg(userId));
                while (userAudioState->frames.size() > 150)
                    userAudioState->frames.pop_front();
            }

            // Setup existing audio channels and users positional state.
            if (userAudioState->soundChannel.get() && allowReceivingPositional)
            {
                // Update positional and position info.
                if (userAudioState->soundChannel->IsPositional() != userAudioState->isPositional)
                    userAudioState->soundChannel->SetPositional(userAudioState->isPositional);
                if (userAudioState->isPositional)
                {
                    // Only update positional data to the channel if it has changed as it does multiple calls to OpenAL.
                    userAudioState->soundChannel->SetRange(static_cast<float>(positionalInnerRange), static_cast<float>(positionalOuterRange), 1.0f);
                    if (!userAudioState->soundChannel->Position().Equals(userAudioState->pos))
                        userAudioState->soundChannel->SetPosition(userAudioState->pos);
                    if (!user->isMe)
                        user->pos = userAudioState->pos;
                }

                // Only emits on change.
                if (!user->isMe)
                    user->SetAndEmitPositional(userAudioState->isPositional);
            }
            else if (userAudioState->soundChannel.get() && !allowReceivingPositional)
            {
                // Reset positional info from the channel
                if (userAudioState->soundChannel->IsPositional())
                    userAudioState->soundChannel->SetPositional(false);

                // Reset users positional state.
                if (!user->isMe && user->isPositional)
                {
                    user->pos = float3::zero;
                    user->SetAndEmitPositional(false);
                }
            }

            // Iterate and play audio frames for this user
            AudioFrameDeque::iterator frameEnd = userAudioState->frames.end();
            for (AudioFrameDeque::iterator frameIter = userAudioState->frames.begin(); frameIter != frameEnd; ++frameIter)
            {
                const SoundBuffer &frame = (*frameIter);
                if (userAudioState->soundChannel.get())
                {
                    // Create new AudioAsset to be added to the sound channels playback buffer.
                    AudioAssetPtr audioAsset = framework->Audio()->CreateAudioAssetFromSoundBuffer(frame);
                    if (audioAsset.get())
                    {
                        // Update user speaking state and add buffer to the sound channel.
                        user->SetAndEmitSpeaking(true); // Only emits on change.
                        userAudioState->soundChannel->AddBuffer(audioAsset);
                    }
                    else
                    {
                        LogWarning(LC + QString("Failed to create new sound buffer for user id %1, clearing all his input frames").arg(userId));

                        // Something went wrong, eg. out of memory, release "broken" SoundChannel and its data.
                        // Bail out as otherwise the next buffer creation will most likely fail the same way.
                        user->SetAndEmitSpeaking(false); // Only emits on change.
                        userAudioState->soundChannel->Stop();
                        userAudioState->soundChannel.reset();
                        break;
                    }
                }
                else
                {
                    // Create sound channel with initial audio frame. We should get here only once per this for loop.
                    userAudioState->soundChannel = framework->Audio()->PlaySoundBuffer(frame, SoundChannel::Voice);
                    if (userAudioState->soundChannel.get())
                    {
                        // Set positional if available and our local settings allows it
                        if (allowReceivingPositional && userAudioState->isPositional)
                        {
                            userAudioState->soundChannel->SetPositional(true);
                            userAudioState->soundChannel->SetRange(static_cast<float>(positionalInnerRange), static_cast<float>(positionalOuterRange), 1.0f);
                            userAudioState->soundChannel->SetPosition(userAudioState->pos);
                            if (!user->isMe)
                            {
                                user->pos = userAudioState->pos;
                                user->SetAndEmitPositional(true);
                            }
                        }
                        else
                        {
                            userAudioState->soundChannel->SetPositional(false);
                            if (!user->isMe && user->isPositional)
                            {
                                user->pos = float3::zero;
                                user->SetAndEmitPositional(false);
                            }
                        }

                        // Update user speaking state. Only emits on change.
                        user->SetAndEmitSpeaking(true);
                    }
                    else
                        LogError("Failed to create sound channel for " + QString::number(userId));
                }
            }

            // Store playback frames for echo cancellation
            if (speexEcho && !localOutputAudioMuted && userAudioState->soundChannel.get())
                userAudioState->playedFrames.insert(userAudioState->playedFrames.end(), userAudioState->frames.begin(), userAudioState->frames.end());
            else
                userAudioState->playedFrames.clear();

            // Clear users input frames
            userAudioState->frames.clear();
        }

        mutexInput.unlock();
    }

    void AudioProcessor::ClearInputAudio()
    {
        // This function should be called in the main thread
        QMutexLocker lock(&mutexInput);
        if (inputAudioStates.size() > 0)
        {
            for (AudioStateMap::iterator iter = inputAudioStates.begin(); iter != inputAudioStates.end(); ++iter)
            {
                UserAudioState *userAudioState = iter->second;
                if (userAudioState)
                    SAFE_DELETE(userAudioState);
            }
            inputAudioStates.clear();
        }
    }

    void AudioProcessor::ClearInputAudio(uint userId)
    {
        // This function should be called in the main thread
        QMutexLocker lock(&mutexInput);

        AudioStateMap::iterator userStateIter = inputAudioStates.find(userId);
        if (userStateIter != inputAudioStates.end())
        {
            UserAudioState *userState = userStateIter->second;
            userState->frames.clear();
            if (userState->soundChannel.get())
                userState->soundChannel->Stop();
            userState->soundChannel.reset();
            SAFE_DELETE(userState);
            inputAudioStates.erase(userStateIter);
        }
    }

    void AudioProcessor::ClearOutputAudio()
    {
        // This function should be called in the main thread
        {
            QMutexLocker lockEncoded(&mutexOutputEncoded);
            pendingEncodedFrames.clear();
            pendingVADPreBuffer.clear();
        }
        {
            QMutexLocker lockOutput(&mutexOutputPCM);
            pendingPCMFrames.clear();
        }
    }

    int AudioProcessor::CodecBitStreamVersion()
    {
        if (this->isRunning())
        {
            LogError(LC + "CodecBitStreamVersion() called when audio thread is running, -1 will be returned!");
            return -1;
        }

        if (!codec)
            return -1;
        else
            return codec->BitStreamVersion();
    }

    void AudioProcessor::PrintCeltError(int celtError, bool decoding)
    {
        // This function is called in the audio thread
        if (celtError == CELT_OK)
            return;

        QString base(LC + (decoding == true ? "celt decoding error: " : "celt encoding error: "));

        switch (celtError)
        {
            case CELT_BAD_ARG:
                LogError(base + "CELT_BAD_ARG");
                break;
            case CELT_INVALID_MODE:
                LogError(base + "CELT_INVALID_MODE");
                break;
            case CELT_INTERNAL_ERROR:
                LogError(base + "CELT_INTERNAL_ERROR");
                break;
            case CELT_CORRUPTED_DATA:
                LogError(base + "CELT_CORRUPTED_DATA");
                break;
            case CELT_UNIMPLEMENTED:
                LogError(base + "CELT_UNIMPLEMENTED");
                break;
            case CELT_INVALID_STATE:
                LogError(base + "CELT_INVALID_STATE");
                break;
            case CELT_ALLOC_FAIL:
                LogError(base + "CELT_ALLOC_FAIL");
                break;
            default:
                LogError(base + "Unknown error enum: " + QString::number(celtError));
                break;
        }
    }

    void AudioProcessor::OnAudioReceived(uint userId, uint seq, ByteArrayVector frames, bool isPositional, float3 pos)
    {
        if (!codec)
            return;

        // This function is called in the audio thread
        if (frames.size() == 0)
            return;

        // This will never* hit if the server was properly informed that we don't want to receive audio,
        // from all or from certain users. See MumblePlugin::SetInputAudioMuted.
        // *The return here will only hit for a short period when input was muted
        // to where the server receives this information and shuts down sending audio to us.
        mutexAudioMute.lockForRead();
        if (inputAudioMuted)
        {
            mutexAudioMute.unlock();
            return;
        }
        mutexAudioMute.unlock();

        QMutexLocker lockBuffers(&mutexInput);

        UserAudioState *userAudioState = 0;
        if (inputAudioStates.find(userId) == inputAudioStates.end())
        {
            userAudioState = new UserAudioState();
            inputAudioStates[userId] = userAudioState;
        }
        else
            userAudioState = inputAudioStates[userId]; // Creates a new one if does not exist already.

        if (userAudioState == 0)
        {
            LogWarning(LC + "User state is null, erasing " + QString::number(userId));
            inputAudioStates.erase(inputAudioStates.find(userId));
            return;
        }

        // If you change audio output settings in Mumble or various other things, sequence will reset to 0.
        // If this is received we need to reset our tracking sequence number as well.
        if (seq == 0)
            userAudioState->lastSeq = 0;

        // If this sequence is older than what has been previously received, ignore the frames
        if (userAudioState->lastSeq > seq)
            return;

        // Update the users audio state struct
        userAudioState->lastSeq = seq;
        userAudioState->isPositional = isPositional;
        if (userAudioState->isPositional)
            userAudioState->pos = pos;

        for (ByteArrayVector::const_iterator frameIter = frames.begin(); frameIter != frames.end(); ++frameIter)
        {
            const QByteArray &inputFrame = (*frameIter);
            SoundBuffer soundFrame;

            int celtResult = userAudioState->codec->Decode(inputFrame.data(), inputFrame.size(), soundFrame);
            if (celtResult == CELT_OK)
                userAudioState->frames.push_back(soundFrame);
            else
            {
                PrintCeltError(celtResult, true);
                userAudioState->frames.clear();
                return;
            }
        }
    }

    void AudioProcessor::OnResetFramesPerPacket()
    {
        mutexAudioSettings.lockForWrite();
        bufferFullFrames = 0;
        qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA;
        mutexAudioSettings.unlock();
    }

    /// UserAudioState

    UserAudioState::UserAudioState()
    {
        codec = new CeltCodec();

        lastSeq = 0;
        isPositional = false;
        pos = float3::zero;

        frames.clear();
        playedFrames.clear();
        soundChannel.reset();
    }

    UserAudioState::~UserAudioState()
    {
        SAFE_DELETE(codec);

        frames.clear();
        playedFrames.clear();
        soundChannel.reset();
    }
}
