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
    AudioProcessor::AudioProcessor(Framework *framework_) :
        LC("[MumbleAudioProcessor]: "),
        framework(framework_),
        codec(new CeltCodec()),
        outputQuality(QualityNotSet)
    {
        SetOutputQuality(QualityBalanced);
    }

    AudioProcessor::~AudioProcessor()
    {
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
            int bufferSize = (MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8) * 20;
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

    void AudioProcessor::SetOutputQuality(MumbleAudio::AudioQuality quality)
    {
        // This function is called in the main thread

        QMutexLocker lock(&mutexAudioQuality);
        switch(quality)
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
            case QualityUltra:
            {
                qualityBitrate = MUMBLE_AUDIO_QUALITY_ULTRA;
                qualityFramesPerPacket = MUMBLE_AUDIO_FRAMES_PER_PACKET_ULTRA;
                break;
            }
        }
    }

    void AudioProcessor::SetFramesPerPacket(int frames)
    {
        // This function is called in the main thread

        if (frames < 1)
            return;

        QMutexLocker lock(&mutexAudioQuality);
        qualityFramesPerPacket = frames;
        ClearOutputAudio();

        LogInfo(LC + "Frames per packet now: " + QString::number(frames));
    }

    QList<QByteArray> AudioProcessor::ProcessOutputAudio()
    {
        // This function is called in the main thread

        if (!framework)
            return QList<QByteArray>();

        QMutexLocker lockCodec(&mutexCodec);
        if (!codec)
            return QList<QByteArray>();

        int localQualityBitrate = 0;
        int localQualityFramesPerPacket = 0;

        {
            QMutexLocker lock(&mutexAudioQuality);
            localQualityBitrate = qualityBitrate;
            localQualityFramesPerPacket = qualityFramesPerPacket;
        }

        QList<QByteArray> outputFrames;
        int totalEncodedBytes = 0;

        uint celtFrameSize = MUMBLE_AUDIO_SAMPLES_IN_FRAME * MUMBLE_AUDIO_SAMPLE_WIDTH / 8;
        while (framework->Audio()->GetRecordedSoundSize() > celtFrameSize)
        {
            SoundBuffer outputPCM;
            outputPCM.data.resize(celtFrameSize);
            
            uint bytesOut = framework->Audio()->GetRecordedSoundData(&outputPCM.data[0], celtFrameSize);
            if (bytesOut == celtFrameSize)
            {
                unsigned char compressedBuffer[512];
                int bytesWritten = codec->Encode(outputPCM, compressedBuffer, localQualityBitrate);
                if (bytesWritten > 0)
                {
                    totalEncodedBytes += bytesWritten;
                    outputFrames.push_back(QByteArray(reinterpret_cast<const char *>(compressedBuffer), bytesWritten));
                }
            }
        }

        // This ensures that when our local queue is getting too big
        // (network cant send fast enough) we reset the situation.
        /// @todo If this happens we should increase the frames per packet automatically?
        /// @todo smarter logic, trim the list instead
        if (outFramesPending.size() > 20)
        {
            LogWarning(LC + "Output local buffer getting too large, reseting situation.");
            ClearOutputAudio();
            return QList<QByteArray>();
        }

        // Nothing was captured and no pending
        if (outputFrames.size() == 0 && outFramesPending.size() > 0)
            return outputFrames;

        // Nothing was captured but we have pending
        if (outputFrames.size() == 0 && outFramesPending.size() >= localQualityFramesPerPacket)
        {
            QList<QByteArray> sendOutNow;
            while (sendOutNow.size() < localQualityFramesPerPacket)
                sendOutNow.push_back(outFramesPending.takeFirst());
            return sendOutNow;
        }

        // No pending frames
        if (outFramesPending.size() == 0 && outputFrames.size() == localQualityFramesPerPacket)
            return outputFrames;

        // Push to pending
        outFramesPending.append(outputFrames);

        // Pull from pending if enough frames available
        QList<QByteArray> sendOutNow;
        if (outFramesPending.size() >= localQualityFramesPerPacket)
        {
            while (sendOutNow.size() < localQualityFramesPerPacket)
                sendOutNow.push_back(outFramesPending.takeFirst());
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
            if (!pendingChannelRemoves.isEmpty())
            {
                foreach(uint channelUserId, pendingChannelRemoves)
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
                pendingChannelRemoves.clear();
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
        // This function is called in the main thread

        QMutexLocker lock(&mutexInput);
        inputFrames.clear();
        userChannels.clear();
    }

    void AudioProcessor::ClearInputAudio(uint userId)
    {
        // This function is called in the main thread

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
        // This function is called in the main thread

        outFramesPending.clear();
    }

    int AudioProcessor::CodecBitStreamVersion()
    {
        // This function is called in the main thread

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
            if (!pendingChannelRemoves.contains(userId))
                pendingChannelRemoves.push_back(userId);
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
