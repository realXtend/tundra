// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CommunicationWidget_h
#define incl_UiModule_CommunicationWidget_h

#include <QGraphicsProxyWidget>
#include "ui_CommunicationWidget.h"

#include <QLabel>

class QStackedLayout;
class QPlainTextEdit;

namespace UiServices
{
    class UiProxyWidget;
}

namespace CoreUi
{
    class NormalChatViewWidget;
    class ChatLabel;

    class CommunicationWidget : public QGraphicsProxyWidget, private Ui::CommunicationWidget
    {

    Q_OBJECT

    public:
        CommunicationWidget();
        enum ViewMode { Normal, History };      

    public slots:
        void UpdateController(QObject *controller);
        void UpdateImWidget(UiServices::UiProxyWidget *im_proxy);
        
    private slots:
        void Initialise();
        void ChangeViewPressed();
        void ChangeView(ViewMode new_mode);
        void ToggleImWidget();

        void ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message);
        void SendMessageRequested();

    private:
        ViewMode viewmode_;

        QWidget *internal_widget_;
        QObject *current_controller_;
        QStackedLayout *stacked_layout_;
        QPlainTextEdit *history_view_text_edit_;
        NormalChatViewWidget *normal_view_widget_;
        UiServices::UiProxyWidget *im_proxy_;

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