// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_VoiceIndicator_h
#define incl_MumbleVoipModule_VoiceIndicator_h

#include <QObject>
#include "PCMAudioFrame.h"

namespace MumbleVoip
{
    class VoiceIndicatorInterface : public QObject
    {
        Q_OBJECT
    public:
        virtual ~VoiceIndicatorInterface() {};
        virtual bool IsSpeaking() = 0;
        virtual double VoiceLevel() = 0;
        virtual void AnalyzeAudioFrame(PCMAudioFrame* frame) = 0;

        // For user settings..
        //virtual void SetTresholdLevel(double);
        //virtual double GetTresholdLevel();

    signals:
        void StartSpeaking();
        void StopSpeaking();
    };

    //! A very simple voice indicator 
    //! 
    //! Analyzes every audio packet for average absolute sample value.
    //! Positive audio packet sets speaking on for a certain time and if
    //! no voice is detect during that time then speaking is set to false.
    class SimpleVoiceIndicator : public VoiceIndicatorInterface
    {
        Q_OBJECT
    public:
        SimpleVoiceIndicator();
        virtual ~SimpleVoiceIndicator();
        virtual bool IsSpeaking();
        virtual double VoiceLevel();
        virtual void AnalyzeAudioFrame(PCMAudioFrame* frame);
    private:
        bool speaking_;
        int last_voice_ms_;
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_VoiceIndicator_h
