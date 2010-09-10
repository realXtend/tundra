// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_VoiceControler_h
#define incl_UiModule_VoiceControler_h

#include <QObject>
#include <QWidget>
#include "ui_VoiceControl.h"

namespace Communications
{
    namespace InWorldVoice
    {
        class SessionInterface;
        class ParticipantInterface;
    }
}

namespace CommUI
{
    class VoiceController : public QObject
    {
        Q_OBJECT
//        Q_PROPERTY(TransmissionMode transmissionMode READ GetTransmissionMode WRITE SetTransmissionMode)

    public:
        enum State { Disabled, Connecting, Connected, ConnectionLost };
        enum TransmissionMode { Mute, ContinuousTransmission, PushToTalk, ToggleMode, VoiceActivity };

        VoiceController(Communications::InWorldVoice::SessionInterface* voice_session);
        virtual ~VoiceController();

    public slots:
        virtual void SetTransmissionMode(TransmissionMode mode);
        virtual void SetPushToTalkOn();
        virtual void SetPushToTalkOff();
        virtual Communications::InWorldVoice::SessionInterface* GetSession() { return in_world_voice_session_; }
        //virtual void SetVoiceActivityOn();
        //virtual void SetVoiceActivityOff();
        //virtual TransmissionMode GetTransmissionMode();
        //virtual void SetTransmissionMode(TransmissionMode mode);
    signals:
        void TransmittingAudioStarted();
        void TransmittingAudioStopped();
        void TransmissionModeChanged(TransmissionMode new_mode);

    protected slots:
        void SetTransmissionState();

    private:
        TransmissionMode transmission_mode_;
        bool transmitting_audio_;
        bool push_to_talk_on_;
        bool toggle_mode_on_;
        int voice_activity_timeout_ms_;
        Communications::InWorldVoice::SessionInterface* in_world_voice_session_;
//        QTime last_voice_activity_time_;
    };

    class VoiceControllerWidget : public QObject, public VoiceController, private Ui::VoiceControl, public QWidget
    {
        Q_OBJECT
    public:
        VoiceControllerWidget(Communications::InWorldVoice::SessionInterface* voice_session);
        virtual ~VoiceControllerWidget();

    private slots:
        void ApplyTransmissionModeSelection(int selection);
        void OpenParticipantListWidget();
        void ApplyMuteAllSelection();
        void UpdateUI();
    };

} // CommUI

#endif // incl_UiModule_VoiceControler_h
