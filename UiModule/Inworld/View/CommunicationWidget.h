// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CommunicationWidget_h
#define incl_UiModule_CommunicationWidget_h

#include <QGraphicsProxyWidget>
#include "ui_CommunicationWidget.h"

#include <QLabel>

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

    //! Presents status with set of status icons.
    //!
    class StateIndicatorWidget : public QPushButton
    {
        Q_OBJECT
    public:
        enum State { STATE_OFFLINE, STATE_ONLINE, STATE_BUSY, STATE_AWAY };
        StateIndicatorWidget(QWidget * parent = 0, Qt::WindowFlags f = 0 );
        virtual void setState(State state);
        virtual State state() const;
    signals:
        void StateChanged();
    private:
        State state_;
    };

    class CommunicationWidget : public QGraphicsProxyWidget, private Ui::CommunicationWidget
    {

    Q_OBJECT

    public:
        CommunicationWidget(Foundation::Framework* framework);
        enum ViewMode { Normal, History };      

    public slots:
        void UpdateController(QObject *controller);
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

        void ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message);
        void SendMessageRequested();
        void InitializeInWorldVoice();
        void UninitializeInWorldVoice();
        void UpdateInWorldVoiceIndicator();

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

        StateIndicatorWidget* voice_status_;

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

#endif