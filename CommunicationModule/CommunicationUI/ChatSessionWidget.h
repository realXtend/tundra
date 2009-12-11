// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_ChatSessionWidget_h
#define incl_Communication_ChatSessionWidget_h

#include "ui_ChatSessionWidget.h"
#include "interface.h"

namespace CommunicationUI
{
    class ChatSessionWidget : public QWidget
    {

    Q_OBJECT

    public:
        ChatSessionWidget(QWidget *parent, Communication::ChatSessionInterface *chat_session, QString &my_name, QString &his_name);
        virtual ~ChatSessionWidget();

    private:
        void ChatSessionWidget::MessageToConversation(QString &formatted_text) { chat_session_ui_.chatTextArea->appendHtml(formatted_text); }
        void ChatSessionWidget::ShowMessageHistory(Communication::ChatMessageVector messageHistory);
        
        Ui::ChatSessionWidget chat_session_ui_;
        Communication::ChatSessionInterface *chat_session_;
        QString my_name_; 
        QString his_name_;

    public slots:
        void MessageReceived(const Communication::ChatMessageInterface &message);
        void ContactStateChanged(const QString &status, const QString &message);

    private slots:
        void SendMessage();
        void CloseConversation();

    signals:
        void Closed(const QString &);

    };
}

#endif // incl_Communication_ChatSessionWidget_h