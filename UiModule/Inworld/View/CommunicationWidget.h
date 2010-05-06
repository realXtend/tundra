// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CommunicationWidget_h
#define incl_UiModule_CommunicationWidget_h

#include <QGraphicsProxyWidget>
#include "ui_CommunicationWidget.h"
#include "ui_VoiceUsers.h"
#include "ui_VoiceUser.h"

#include <QLabel>
#include <QTimer>

class QStackedLayout;
class QPlainTextEdit;
class QGraphicsSceneMouseEvent;

namespace UiServices
{
    class UiProxyWidget;
}

namespace Communications
{
    namespace InWorldVoice
    {
        class SessionInterface;
        class ParticipantInterface;
    }
}

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class NormalChatViewWidget;
    class ChatLabel;

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

    //! Presents InWorldVoice::Participant object
    class VoiceUserWidget : public QWidget, private Ui::voiceUserWidget
    {
        Q_OBJECT
    public:
        VoiceUserWidget(Communications::InWorldVoice::ParticipantInterface* participant);
    private:
        Communications::InWorldVoice::ParticipantInterface* participant_;
    };

    //! VoiceUserListWidget
    class VoiceUsersWidget : public QWidget, private Ui::VoiceUsersWidget
    {
        Q_OBJECT
    public:
        VoiceUsersWidget(QWidget *parent = 0, Qt::WindowFlags wFlags = 0);
        virtual void SetSession(Communications::InWorldVoice::SessionInterface* session);
    public slots:
        void UpdateList();
    private:
        Communications::InWorldVoice::SessionInterface* session_;
        QList<VoiceUserWidget *> user_widgets_;
    };

    //! Provide communications functionalities to end user
    //! CommunicationWidget is located to bottom left corner view.
    class CommunicationWidget : public QGraphicsProxyWidget, private Ui::CommunicationWidget
    {
        Q_OBJECT
    public:
        CommunicationWidget(Foundation::Framework* framework);
        enum ViewMode { Normal, History };      

    public slots:
        void UpdateController(QObject *controller);
        void UpdateImWidget(UiServices::UiProxyWidget *im_proxy);
//        void UpdateVoiceUsersWidget(UiServices::UiProxyWidget *im_proxy);

        void SetFocusToChat();
        
    protected:
        void hoverMoveEvent(QGraphicsSceneHoverEvent *mouse_hover_move_event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event);
        void mousePressEvent(QGraphicsSceneMouseEvent *mouse_press_event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouse_release_event);

    private slots:
        void Initialise();
        void ChangeViewPressed();
        void ChangeView(ViewMode new_mode);
        void ToggleImWidget();
        void ToggleVoice();
        void ToggleVoiceUsers();

        void ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message);
        void SendMessageRequested();
        void InitializeInWorldVoice();
        void UninitializeInWorldVoice();
        void UpdateInWorldVoiceIndicator();
        void ShowVoiceControls();
        void HideVoiceControls();

    private:
        Foundation::Framework* framework_;
        ViewMode viewmode_;

        QWidget *internal_widget_;
        QObject *current_controller_;
        QStackedLayout *stacked_layout_;
        QPlainTextEdit *history_view_text_edit_;
        NormalChatViewWidget *normal_view_widget_;
        UiServices::UiProxyWidget *im_proxy_;
        Communications::InWorldVoice::SessionInterface* in_world_voice_session_;
        bool in_world_speak_mode_on_;

        QPointF press_pos_;
        QPointF release_pos_;
        bool resizing_vertical_;
        bool resizing_horizontal_;

        // in-world voice
        VoiceStateWidget* voice_state_widget_;
        VoiceUsersInfoWidget* voice_users_info_widget_;
        VoiceUsersWidget* voice_users_widget_;
        UiServices::UiProxyWidget* voice_users_proxy_widget_;

    signals:
        void SendMessageToServer(const QString &message);
    };

    class NormalChatViewWidget : public QWidget
    {

    Q_OBJECT

    public:
        NormalChatViewWidget(QWidget *parent);

    public slots:
        void ShowChatMessage(bool own_message, QString message);

    private slots:
        void RemoveChatLabel(ChatLabel *label);

    };

    class ChatLabel : public QLabel
    {
    
    Q_OBJECT
    
    public:
        ChatLabel(bool own_message, QString message);

    private slots:
        void TimeOut();

    signals:
        void DestroyMe(ChatLabel *);
    };
}

#endif // incl_UiModule_CommunicationWidget_h