// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_FriendListWidget_h
#define incl_Communication_FriendListWidget_h

//#include "FriendHelper.h"
//#include "SessionHelper.h"
//#include "FriendListItem.h"
#include "UiDefines.h"
#include "ui_FriendListWidget.h"

//#include "interface.h"
#include "CommunicationModuleFwd.h"

namespace Foundation
{
    class Framework;
}

namespace CommunicationUI
{
    class FriendListWidget : public QWidget
    {
    
    Q_OBJECT

    public:
        FriendListWidget(Communication::ConnectionInterface *im_connection, UiHelpers::SessionHelper *session_helper,  Foundation::Framework *framework);
        virtual ~FriendListWidget();

        void SetStatus(const QString &status_code);
        QString GetStatus() { return friend_list_ui_->statusEditor->currentText(); }

    public slots:
        void FriendClicked(QListWidgetItem *friend_item);

    private:
        void InitializeFriendList();
        void InitializeStatusEditor();

        Ui::FrienListWidget *friend_list_ui_;
        UiHelpers::FriendHelper *friend_helper_;
        UiHelpers::SessionHelper *session_helper_;

        Communication::ConnectionInterface *im_connection_;
        CommunicationUI::FriendListItem *clicked_item_;
        QMap<QString, Communication::ContactInterface*> contacts_map_;
        QTabWidget *session_tab_widget_;

    private slots:
        void StatusChange(const QString &item_string);
        void StartChatSession();
        void StartVideoSession();
        void RemoveContact();

    signals:
        void NewChatSessionStarted(Communication::ChatSessionInterface *, QString &);
        void NewVideoSessionStarted(Communication::VoiceSessionInterface *, QString &);
        void StatusChanged(const QString &);

    };
}

#endif // incl_Communication_FriendListWidget_h