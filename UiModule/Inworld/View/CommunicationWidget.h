// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CommunicationWidget_h
#define incl_UiModule_CommunicationWidget_h

#include <QGraphicsProxyWidget>
#include "ui_CommunicationWidget.h"

class QStackedLayout;
class QTextBrowser;
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

    namespace InWorldChat
    {
        class SessionInterface;
        class TextMessageInterface;
    }
}

namespace CommUI
{
    class VoiceUsersWidget;
    class VoiceUsersInfoWidget;
    class VoiceStateWidget;
}

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class NormalChatViewWidget;
    class ChatLabel;

    //! Provide communications functionalities to end user
    //! CommunicationWidget is located to bottom left corner view.
    class CommunicationWidget : public QGraphicsProxyWidget, private Ui::CommunicationWidget
    {
        Q_OBJECT

    public:
        CommunicationWidget(Foundation::Framework* framework);
        enum ViewMode { Normal, History };

    public slots:
        void UpdateImWidget(UiServices::UiProxyWidget *im_proxy);
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
        void InitializeInWorldChat();
        void UninitializeInWorldVoice();
        void UpdateInWorldVoiceIndicator();
        void ShowVoiceControls();
        void HideVoiceControls();
        void UpdateInWorldChatView(const Communications::InWorldChat::TextMessageInterface &message);

    private:
        Foundation::Framework* framework_;
        ViewMode viewmode_;

        QWidget *internal_widget_;
        QStackedLayout *stacked_layout_;
        QTextBrowser *history_view_text_edit_;
        NormalChatViewWidget *normal_view_widget_;
        UiServices::UiProxyWidget *im_proxy_;
        Communications::InWorldVoice::SessionInterface* in_world_voice_session_;
        static bool in_world_speak_mode_on_;

        QPointF press_pos_;
        QPointF release_pos_;
        bool resizing_vertical_;
        bool resizing_horizontal_;

        // in-world voice
        CommUI::VoiceStateWidget* voice_state_widget_;
        CommUI::VoiceUsersInfoWidget* voice_users_info_widget_;
        CommUI::VoiceUsersWidget* voice_users_widget_;
        QGraphicsProxyWidget* voice_users_proxy_widget_;

        Communications::InWorldChat::SessionInterface* in_world_chat_session_;

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
