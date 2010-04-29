// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "VoiceIndicator.h"
#include "MumbleVoipModule.h"
#include "PCMAudioFrame.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{

    SimpleVoiceIndicator::SimpleVoiceIndicator() :
        speaking_(false),
        last_voice_ms_(0)
    {
    }

    SimpleVoiceIndicator::~SimpleVoiceIndicator()
    {
    }
    
    bool SimpleVoiceIndicator::IsSpeaking()
    {
        return speaking_;
    }

    double SimpleVoiceIndicator::VoiceLevel()
    {
        return 0; // \todo Implement
    }

    void SimpleVoiceIndicator::AnalyzeAudioFrame(PCMAudioFrame* frame)
    {
        int average_treshold = 150;
        bool was_speaking = speaking_;
        int voice_timeout_ms = 150;
        int average = 0;

        if (frame->SampleWidth() == 16)
        {
            short* data = (short*)frame->DataPtr();
            int samples = frame->Samples();
            for(int i = 0; i < samples; ++i)
            {
                short sample = data[i];
                average += abs(sample);
            }
            average /= samples;
            if (average > average_treshold)
            {
                // Did notice voice activity
                speaking_ = true;
                last_voice_ms_ = 0;
                if (!was_speaking)
                {
                    emit VoiceIndicatorInterface::StartSpeaking();
                }
            }
            else
            {
                // Didn't notice any voice activity 
                last_voice_ms_ += frame->GetLengthMs();
                if (last_voice_ms_ > voice_timeout_ms)
                {
                    speaking_ = false;
                    if (was_speaking)
                    {
                        emit VoiceIndicatorInterface::StopSpeaking();
                    }
                }
            }
        }
    }

} // MumbleVoip
