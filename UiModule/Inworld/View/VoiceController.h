// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_VoiceControler_h
#define incl_UiModule_VoiceControler_h

#include <QObject>
#include <QWidget>
#include <QPoint>
#include <QTimer>
#include "ui_VoiceControl.h"

class QMouseEvent;

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
        virtual void Toggle();
        virtual Communications::InWorldVoice::SessionInterface* GetSession() { return in_world_voice_session_; }
//        virtual TransmissionMode GetTransmissionMode();

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
    };

    class VoiceControllerWidget : public QObject, public QWidget, private Ui::VoiceControl, public VoiceController
    {
        Q_OBJECT
    public:
        VoiceControllerWidget(Communications::InWorldVoice::SessionInterface* voice_session);
        virtual ~VoiceControllerWidget();

    public slots:
        virtual void SetPushToTalkOn();
        virtual void SetPushToTalkOff();
        virtual void Toggle();

    protected:
        virtual void mouseMoveEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseReleaseEvent(QMouseEvent *);

    private slots:
        void ApplyTransmissionModeSelection(int selection);
        void OpenParticipantListWidget();
        void ApplyMuteAllSelection();
        void UpdateUI();

    private:
        QPoint mouse_last_pos_;
        QTimer update_timer_;
        bool mouse_dragging_;
    };

} // CommUI

#endif // incl_UiModule_VoiceControler_h
