// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CommunicationWidget.h"
#include "UiProxyWidget.h"

#include <QWidget>
#include <QStackedLayout>
#include <QPlainTextEdit>
#include <QTimer>

namespace CoreUi
{
    CommunicationWidget::CommunicationWidget() :
        QGraphicsProxyWidget(),
        internal_widget_(new QWidget()),
        current_controller_(0),
        im_proxy_(0),
        viewmode_(Normal)
    {
        Initialise();
        ChangeView(viewmode_);
    }

    // Private

    void CommunicationWidget::Initialise()
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
        imButton->setEnabled(false);

        // Stacked layout
        stacked_layout_ = new QStackedLayout(chatContentWidget);
        stacked_layout_->setMargin(0);
        contentContainerLayout->addLayout(stacked_layout_);

        // History view mode
        history_view_text_edit_ = new QPlainTextEdit(chatContentWidget);
        history_view_text_edit_->setReadOnly(true);
        history_view_text_edit_->setObjectName("historyViewTextEdit");
        history_view_text_edit_->setStyleSheet("QPlainTextEdit#historyViewTextEdit { background-color: rgba(34,34,34,191); border-radius: 7px; border: 1px solid rgba(255,255,255,50); }");
        history_view_text_edit_->setFont(QFont("Calibri", 11));
        stacked_layout_->addWidget(history_view_text_edit_);

        // Slim view mode
        normal_view_widget_ = new NormalChatViewWidget(chatContentWidget);
        stacked_layout_->addWidget(normal_view_widget_);

        stacked_layout_->setCurrentWidget(normal_view_widget_);

        connect(viewModeButton, SIGNAL( clicked() ), SLOT( ChangeViewPressed() ));
        connect(imButton, SIGNAL( clicked() ), SLOT( ToggleImWidget() ));
        connect(chatLineEdit, SIGNAL( returnPressed() ), SLOT( SendMessageRequested() ));
    }

    void CommunicationWidget::ChangeViewPressed()
    {
        switch (viewmode_)
        {
            case Normal:
                ChangeView(History);
                break;
            case History:
                ChangeView(Normal);
                break;
        }
    }

    void CommunicationWidget::ChangeView(ViewMode new_mode)
    {
        viewmode_ = new_mode; 
        switch (viewmode_)
        {
            case Normal:
                chatContentWidget->setStyleSheet("QWidget#chatContentWidget { background-color: transparent; border-top-left-radius: 0px; border-top-right-radius: 0px; }");
                viewModeButton->setStyleSheet("QPushButton#viewModeButton { background-image: url('./data/ui/images/chat/uibutton_HISTORY_normal.png'); }");
                stacked_layout_->setCurrentWidget(normal_view_widget_);
                break;
            case History:
                chatContentWidget->setStyleSheet("QWidget#chatContentWidget { background-color: rgba(34,34,34,191); border-top-left-radius: 7px; border-top-right-radius: 7px; }");
                viewModeButton->setStyleSheet("QPushButton#viewModeButton { background-image: url('./data/ui/images/chat/uibutton_HISTORY_click.png'); }");
                stacked_layout_->setCurrentWidget(history_view_text_edit_);
                break;
        }
    }

    void CommunicationWidget::ToggleImWidget()
    {
        if (im_proxy_)
        {
            if (!im_proxy_->isVisible())
                im_proxy_->show();
            else
                im_proxy_->AnimatedHide();
        }
    }

    void CommunicationWidget::ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message)
    {
        // History view
        timestamp = timestamp.midRef(timestamp.indexOf(" ")+1).toString(); // Cut the fat from timestamp for now
        QString htmlcontent("<span style='color:grey'>[");
        htmlcontent.append(timestamp);
        if (!self_sent_message)
            htmlcontent.append("]</span> <span style='color:#0099FF;'>");
        else
            htmlcontent.append("]</span> <span style='color:#FF3330;'>");
        htmlcontent.append(sender);
        htmlcontent.append(": </span><span style='color:#EFEFEF;'>");
        htmlcontent.append(message);
        htmlcontent.append("</span>");
        history_view_text_edit_->appendHtml(htmlcontent);

        // Normal view
        if (!self_sent_message)
            normal_view_widget_->ShowChatMessage(self_sent_message, QString("%1: %2").arg(sender, message));
        else
            normal_view_widget_->ShowChatMessage(self_sent_message, QString("Me: %1").arg(message));
    }

    void CommunicationWidget::SendMessageRequested()
    {
        if (chatLineEdit->text().isEmpty())
            return;

        QString message = chatLineEdit->text();
        chatLineEdit->clear();
        emit SendMessageToServer(message);
    }

    // Public

    void CommunicationWidget::UpdateController(QObject *controller)
    {
        // Disconnect previous
        if (current_controller_)
        {
            this->disconnect(SIGNAL( SendMessageToServer(const QString&) ));
            this->disconnect(current_controller_);
        }

        // Connect present controller
        current_controller_ = controller;
        connect(current_controller_, SIGNAL( MessageReceived(bool, QString, QString, QString) ),
                this, SLOT( ShowIncomingMessage(bool, QString, QString, QString) ));
        connect(this, SIGNAL( SendMessageToServer(const QString&) ), 
                current_controller_, SLOT( SendChatMessageToServer(const QString&) ));

        // Clear old ui messages from history view
        if (history_view_text_edit_)
            history_view_text_edit_->clear();
    }

    void CommunicationWidget::UpdateImWidget(UiServices::UiProxyWidget *im_proxy)
    {
        im_proxy_ = im_proxy;
        imButton->setEnabled(true);
    }

    // NormalChatViewWidget : QWidget

    NormalChatViewWidget::NormalChatViewWidget(QWidget *parent) :
        QWidget(parent)
    {
        setObjectName("normalChatViewWidget");
        setStyleSheet("QWidget#normalChatViewWidget { background-color: transparent; }");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(3);
        layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
        setLayout(layout);
    }

    void NormalChatViewWidget::ShowChatMessage(bool own_message, QString message)
    {
        ChatLabel *chat_label = new ChatLabel(own_message, message);
        layout()->addWidget(chat_label);
        connect(chat_label, SIGNAL( DestroyMe(ChatLabel*) ), SLOT( RemoveChatLabel(ChatLabel*) ));
    }

    void NormalChatViewWidget::RemoveChatLabel(ChatLabel *label)
    {
        int index = layout()->indexOf(label);
        if (index != -1)
        {
            layout()->removeItem(layout()->itemAt(index));
            SAFE_DELETE(label);
            if (layout()->count() < 4)
                updateGeometry();
        }
    }

    // ChatLabel

    ChatLabel::ChatLabel(bool own_message, QString message) :
        QLabel(message)
    {
        setFont(QFont("Arial", 12));
        if (own_message)
            setStyleSheet("background-color: rgba(34,34,34,191); color: white; border-radius: 5px; padding: 3px;");
        else
            setStyleSheet("background-color: rgba(34,34,34,150); color: white; border-radius: 5px; padding: 3px;");
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QTimer::singleShot(10000, this, SLOT(TimeOut()));
    }

    void ChatLabel::TimeOut()
    {
        emit DestroyMe(this);
    }

}