// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_VoiceControl_h
#define incl_UiModule_VoiceControl_h

#include <QPushButton>
#include <QLabel>
#include <QTimer>

namespace CommUI
{
    //namespace InWorldVoice
    //{
    //    class SessionInterface;
    //    class ParticipantInterface;
    //}

    //! Presents state with a set of status icons.
    //!
    //! @todo: Move to out from UModule
    //! @todo: Rename to VoiceStateButton
    class VoiceStateWidget : public QPushButton
    {
        Q_OBJECT
    public:
        enum State { STATE_OFFLINE, STATE_ONLINE };
        VoiceStateWidget(QWidget * parent = 0, Qt::WindowFlags f = 0 );
        virtual void setState(State state);
        virtual State state() const;
        virtual void SetVoiceActivity(double activity);
    signals:
        void StateChanged();
    private:
        void UpdateStyleSheet(); 
        static const int VOICE_ACTIVITY_UPDATE_INTERVAL_MS_ = 50;
        static const int VOICE_ACTIVITY_FADEOUT_MAX_MS_ = 500;

        State state_;
        double voice_activity_;
        QTimer update_timer_;
    private slots:
        void UpdateVoiceActivity();
    };

    //! Show user count and voice activity
    //!
    //! @todo: Move to out from UModule
    //! @todo Rename to VoiceUsersButton
    class VoiceUsersInfoWidget : public QPushButton
    {
        Q_OBJECT
    public:
        VoiceUsersInfoWidget(QWidget* parent = 0);
        virtual void SetUsersCount(int count);
        virtual int UsersCount() const;
        virtual void SetVoiceActivity(double activity);
    private:
        void UpdateStyleSheet();

        int user_count_;
        double voice_activity_;
        QLabel count_label_;
    };

    //! Provides in-world voice status information and controls
    class VoiceControlWidget 
    {
        //! @todo IMPLEMENT
    };

} // CommUI

#endif // incl_UiModule_VoiceControl_h
