// For conditions of distribution and use, see copyright notice in LICENSE

#include "AudioProcessor.h"
#include "CeltCodec.h"
#include "celt/celt.h"

#include "AudioAPI.h"
#include "Framework.h"
#include "CoreDefines.h"
#include "LoggingFunctions.h"

#include <QMutexLocker>

namespace MumbleAudio
{
    AudioProcessor::AudioProcessor(Framework *framework_, MumbleAudio::AudioSettings settings) :
        LC("[MumbleAudioProcessor]: "),
        framework(framework_),
        codec(new CeltCodec()),
        speexPreProcessor(0),
        outputPreProcessed(false),
        preProcessorReset(true),
        isSpeech(false),
        wasPreviousSpeech(false),
        holdFrames(0)
    {
        ApplySettings(settings);
    }

    AudioProcessor::~AudioProcessor()
    {
    }

    void AudioProcessor::ResetSpeexProcessor()
    {
        if (!preProcessorReset)
            return;

        QMutexLocker lock(&mutexAudioSettings);

        outputPreProcessed = false;
        if (audioSettings.suppression < 0 || audioSettings.amplification > 0)
            outputPreProcessed = true;

        if (speexPreProcessor)
            speex_preprocess_state_destroy(speexPreProcessor);

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

        float v = 30000.0f / static_cast<float>(audioSettings.amplification);
        arg = static_cast<int>(floorf(20.0f * log10f(v)));
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &arg);

        arg = 12;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_INCREMENT, &arg);

        arg = -60;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_AGC_DECREMENT, &arg);
        
        arg = audioSettings.suppression;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &arg);

        fArg = 0.0f;
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &fArg);
        speex_preprocess_ctl(speexPreProcessor, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &fArg);
    }

    void AudioProcessor::run()
    {
        exec(); // Blocks untill quit()

        {
            QMutexLocker lockCodec(&mutexCodec);
            SAFE_DELETE(codec);
        }

        {
            QMutexLocker lockInput(&mutexInput);
            inputFrames.clear();
            userChannels.clear();
            framework = 0;
        }

        if (speexPreProcessor)
            speex_preprocess_state_destroy(speexPreProcessor);
    }

    void AudioProcessor::SetOutputAudioMuted(bool outputAudioMuted_)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        {
            QMutexLocker lock(&mutexAudioMute);
            if (outputAudioMuted == outputAudioMuted_)
                return;
            outputAudioMuted = outputAudioMuted_;
        }

        if (!outputAudioMuted)
        {
            // Recording buffer 19200 bytes
            int bufferSize = (MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8) * 40;
            framework->Audio()->StartRecording("", MUMBLE_AUDIO_SAMPLE_RATE, true, false, bufferSize);
        }
        else
            framework->Audio()->StopRecording();

        ClearOutputAudio();
    }

    void AudioProcessor::SetInputAudioMuted(bool inputAudioMuted_)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        {
            QMutexLocker lock(&mutexAudioMute);
            if (inputAudioMuted == inputAudioMuted_)
                return;
            inputAudioMuted = inputAudioMuted_;
        }
    }

    void AudioProcessor::ApplySettings(AudioSettings settings)
    {
        // This function is called in the main thread
        {
            QMutexLocker lock(&mutexAudioSettings);

            audioSettings = settings;
            if (audioSettings.suppression > 0)
                audioSettings.suppression = 0;

            switch(audioSettings.quality)
            {
                case QualityLow:
                {
                    qualityBitrate = MUMBLE_AUDIO_QUALITY_LOW;
                    qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_LOW;
                    break;
                }
                case QualityBalanced:
                {
                    qualityBitrate = MUMBLE_AUDIO_QUALITY_BALANCED;
                    qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_BALANCED;
                    break;
                }
                case QualityHigh:
                {
                    qualityBitrate = MUMBLE_AUDIO_QUALITY_ULTRA;
                    qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA;
                    break;
                }
            }
        }

        preProcessorReset = true;
        ResetSpeexProcessor();
    }

    MumbleAudio::AudioSettings AudioProcessor::GetSettings()
    {
        QMutexLocker lock(&mutexAudioSettings);
        return audioSettings;
    }

    QList<QByteArray> AudioProcessor::ProcessOutputAudio()
    {
        // This function is called in the main thread
        if (!framework)
            return QList<QByteArray>();

        // Get recorded PCM frames
        QList<SoundBuffer> pcmFrames;
        uint celtFrameSize = MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8;
        while (framework->Audio()->GetRecordedSoundSize() > celtFrameSize)
        {
            SoundBuffer outputPCM;
            outputPCM.data.resize(celtFrameSize);
            uint bytesOut = framework->Audio()->GetRecordedSoundData(&outputPCM.data[0], celtFrameSize);
            if (bytesOut == celtFrameSize)
                pcmFrames.push_back(outputPCM);
        }

        // Encoded frames per packet
        int framesPerPacket = 0;

        // Preprocess with speexdsp and encode with celt
        QList<QByteArray> encodedFrames;
        if (pcmFrames.size() > 0)
        {
            int localQualityBitrate = 0;
            int localSuppress = 0;
            int localGain = 0;
            bool detectVAD = false;
            float VADmin = 0.0;
            float VADmax = 0.0;
            bool localPreProcess = false;

            {
                QMutexLocker lock(&mutexAudioSettings);
                localQualityBitrate = qualityBitrate;
                framesPerPacket = qualityFramesPerPacket;
                localPreProcess = outputPreProcessed;
                localSuppress = audioSettings.suppression;
                detectVAD = audioSettings.transmitMode == TransmitVoiceActivity;
                VADmin = audioSettings.VADmin;
                VADmax = audioSettings.VADmax;
            }

            QMutexLocker lockCodec(&mutexCodec);
            if (!codec)
                return QList<QByteArray>();

            foreach(SoundBuffer pcmFrame, pcmFrames)
            {
                isSpeech = true;
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

                        // Hold certain amount of frames even if not speaking.
                        // This allows end of sentences to get to the outgoing buffer safely.
                        if (isSpeech)
                            holdFrames = 0;
                        else
                        {
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
                    // append any pre buffered frames so start of sentences
                    // can get to the outgoing buffer safely.
                    if (isSpeech || wasPreviousSpeech)
                    {
                        if (pendingVADPreBuffer.size() > 0)
                        {
                            encodedFrames.append(pendingVADPreBuffer);
                            pendingVADPreBuffer.clear();
                        }
                        encodedFrames.push_back(encodedFrame);
                    }
                    // If voice activity detection is enabled but this is 
                    // not speech, add the frame to the VAD pre buffer.
                    else if (detectVAD && !isSpeech && !wasPreviousSpeech)
                    {
                        if (pendingEncodedFrames.size() > 0)
                            pendingEncodedFrames.clear();
                        while(pendingVADPreBuffer.size() >= (qualityFramesPerPacket*2))
                            pendingVADPreBuffer.takeFirst();
                        pendingVADPreBuffer.push_back(encodedFrame);
                    }
                }
                wasPreviousSpeech = isSpeech;
            }
        }
        // No frames was read from the microphone, 
        // we still need the amount of frames per packet 
        // to determine how many encoded frames to return.
        else
        {
            QMutexLocker lock(&mutexAudioSettings);
            framesPerPacket = qualityFramesPerPacket;
        }

        // This ensures that when our local queue is getting too big
        // (network cant send fast enough) we reset the situation.
        /// @todo If this happens we should increase the frames per packet automatically?
        /// @todo smarter logic, trim the list instead
        if (pendingEncodedFrames.size() > 20)
        {
            LogWarning(LC + "Output local buffer getting too large, reseting situation.");
            ClearOutputAudio();
            return QList<QByteArray>();
        }

        // Nothing was captured and no pending
        if (encodedFrames.size() == 0 && pendingEncodedFrames.size() == 0)
            return QList<QByteArray>();

        // Nothing was captured but we have pending
        if (encodedFrames.size() == 0 && pendingEncodedFrames.size() >= framesPerPacket)
        {
            QList<QByteArray> sendOutNow;
            while (sendOutNow.size() < framesPerPacket)
                sendOutNow.push_back(pendingEncodedFrames.takeFirst());
            return sendOutNow;
        }

        // No pending frames, but got exact amount on this iteration so no need to pend
        if (pendingEncodedFrames.size() == 0 && encodedFrames.size() == framesPerPacket)
            return encodedFrames;

        // Push current to pending
        pendingEncodedFrames.append(encodedFrames);

        // Pull from pending if enough frames available
        QList<QByteArray> sendOutNow;
        if (pendingEncodedFrames.size() >= framesPerPacket)
        {
            while (sendOutNow.size() < framesPerPacket)
                sendOutNow.push_back(pendingEncodedFrames.takeFirst());
        }
        return sendOutNow;
    }

    void AudioProcessor::PlayInputAudio(QList<uint> mutedUserIds)
    {
        // This function is called in the main thread
        if (!framework)
            return;

        // Remove pending channels, the remove operations are 
        // made to the pending list from the audio thread
        {
            QMutexLocker lockChannels(&mutexAudioChannels);
            if (!pendingSoundChannelRemoves.isEmpty())
            {
                foreach(uint channelUserId, pendingSoundChannelRemoves)
                {
                    AudioChannelMap::iterator channelIter = userChannels.find(channelUserId);
                    if (channelIter != userChannels.end())
                    {
                        if (channelIter->second.get())
                        {
                            channelIter->second->Stop();
                            channelIter->second.reset();
                        }
                        userChannels.erase(channelIter);
                    }
                }
                pendingSoundChannelRemoves.clear();
            }
        }

        QMutexLocker lock(&mutexInput);
        if (inputFrames.empty())
            return;

        AudioFrameMap::iterator iter = inputFrames.begin();
        AudioFrameMap::iterator end = inputFrames.end();
        while(iter != end)
        {
            uint userId = iter->first;
            AudioFrameDeque &userFrames = iter->second;

            iter++;

            if (userFrames.empty())    
                continue;

            // If user is muted skip playback and remove frames
            if (mutedUserIds.contains(userId))
            {
                userFrames.clear();
                continue;
            }

            // Find existing SoundChannel for user
            SoundChannelPtr soundChannel;
            AudioChannelMap::iterator channelIter = userChannels.find(userId);
            if (channelIter != userChannels.end())
            {
                if (channelIter->second.get())
                    soundChannel = channelIter->second;
            }

            // Iterate audio frames for user
            AudioFrameDeque::iterator frameIter = userFrames.begin();
            AudioFrameDeque::iterator frameEnd = userFrames.end();
            while(frameIter != frameEnd)
            {
                const SoundBuffer &frame = (*frameIter);
                if (soundChannel.get())
                {
                    // Add buffers to the sound channel
                    AudioAssetPtr audioAsset = framework->Audio()->CreateAudioAssetFromSoundBuffer(frame);
                    if (audioAsset.get())
                        soundChannel->AddBuffer(audioAsset);
                    // Something went wrong, remove broken SoundChannel
                    else
                    {
                        AudioChannelMap::iterator channelIterErase = userChannels.find(userId);
                        if (channelIterErase != userChannels.end())
                        {
                            if (channelIterErase->second.get())
                            {
                                channelIterErase->second->Stop();
                                channelIterErase->second.reset();
                            }
                            userChannels.erase(channelIterErase);
                        }
                    }
                }
                else
                {
                    // Create sound channel with initial audio frame
                    soundChannel = framework->Audio()->PlaySoundBuffer(frame, SoundChannel::Voice);
                    userChannels[userId] = soundChannel;
                }
                frameIter++;
            }

            // Clear users input frames
            userFrames.clear();
        }
    }
    
    void AudioProcessor::ClearInputAudio()
    {
        // This function should be called in the main thread
        QMutexLocker lock(&mutexInput);
        inputFrames.clear();
        userChannels.clear();
    }

    void AudioProcessor::ClearInputAudio(uint userId)
    {
        // This function should be called in the main thread
        QMutexLocker lock(&mutexInput);
        
        AudioFrameMap::iterator frameIter = inputFrames.find(userId);
        if (frameIter != inputFrames.end())
        {
            frameIter->second.clear();
            inputFrames.erase(frameIter);
        }

        AudioChannelMap::iterator channelIter = userChannels.find(userId);
        if (channelIter != userChannels.end())
        {
            if (channelIter->second.get())
            {
                channelIter->second->Stop();
                channelIter->second.reset();
            }
            userChannels.erase(channelIter);
        }
    }

    void AudioProcessor::ClearOutputAudio()
    {
        // This function should be called in the main thread
        pendingEncodedFrames.clear();
    }

    int AudioProcessor::CodecBitStreamVersion()
    {
        QMutexLocker lock(&mutexCodec);
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

    void AudioProcessor::OnAudioReceived(uint userId, QList<QByteArray> frames)
    {
        // This function is called in the audio thread
        if (frames.isEmpty())
            return;
        
        {
            // This will never* hit if the server was properly informed that we don't want to receive audio,
            // from all or from certain users. See MumblePlugin::SetInputAudioMuted.
            // *The return here will only hit for a short period when input was muted
            // to where the server receives this information and shuts down sending audio to us.
            QMutexLocker lockAudio(&mutexAudioMute);
            if (inputAudioMuted)
                return;
        }

        QMutexLocker lockBuffers(&mutexInput);
        AudioFrameDeque &userFrames = inputFrames[userId];

        // Check frame counts, clear input frames and pending frames from SoundChannel.
        // This happens when main thread is blocked from reading the queued frames but the network thread
        // is still filling the frame queue. If we release too much frames to AudioAPI/OpenAL it will eventually crash.
        if ((userFrames.size() + frames.size()) > 10)
        {
            userFrames.clear();
            QMutexLocker lockChannels(&mutexAudioChannels);
            if (!pendingSoundChannelRemoves.contains(userId))
                pendingSoundChannelRemoves.push_back(userId);
            return;
        }

        // Lock codec and check if it still alive, might happen after quit()
        QMutexLocker lockCodec(&mutexCodec);
        if (!codec)
            return;

        foreach(QByteArray frame, frames)
        {
            SoundBuffer soundFrame;

            int celtResult = codec->Decode(frame.data(), frame.size(), soundFrame);
            if (celtResult == CELT_OK)
                userFrames.push_back(soundFrame);
            else
            {
                PrintCeltError(celtResult, true);
                userFrames.clear();
                return;
            }            
        }
    }
}
