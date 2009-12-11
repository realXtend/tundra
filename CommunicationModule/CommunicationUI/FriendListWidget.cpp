// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "FriendListWidget.h"

#include <QDebug>
#include <QMenu>

namespace CommunicationUI
{
    FriendListWidget::FriendListWidget(Communication::ConnectionInterface *im_connection, UiHelpers::SessionHelper *session_helper)
        : QWidget(),
          im_connection_(im_connection),
          friend_helper_(new UiHelpers::FriendHelper),
          session_helper_(session_helper),
          friend_list_ui_(new Ui::FrienListWidget),
          clicked_item_(0)
    {
        hide();
        friend_list_ui_->setupUi(this);
        friend_helper_->SetupUi(friend_list_ui_);

        InitializeFriendList();
        InitializeStatusEditor();

        // Own connections for showing friend menu and selecting status
        connect(friend_list_ui_->friendListWidget, SIGNAL( itemClicked(QListWidgetItem *) ), SLOT( FriendClicked(QListWidgetItem *) ));
        connect(friend_list_ui_->statusEditor, SIGNAL( currentIndexChanged(const QString &) ), this, SLOT( StatusChange(const QString &) ));

        // Helper connections for contacts related slots
        connect(im_connection_, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), 
                friend_helper_, SLOT( NewFriendRequest(Communication::FriendRequestInterface&) ));
        connect(im_connection_, SIGNAL( NewContact(const Communication::ContactInterface&) ), 
                friend_helper_, SLOT( OnNewContact(const Communication::ContactInterface&) ));
        connect(im_connection_, SIGNAL( ContactRemoved(const Communication::ContactInterface&) ), 
                friend_helper_, SLOT( OnContactRemoved(const Communication::ContactInterface&) ));
    }

    FriendListWidget::~FriendListWidget()
    {
        SAFE_DELETE(friend_list_ui_);
        SAFE_DELETE(friend_helper_);
    }

    void FriendListWidget::InitializeFriendList()
    {
        Communication::ContactVector contacts = im_connection_->GetContacts().GetContacts();
        Communication::ContactVector::const_iterator iter;

        for( iter=contacts.begin(); iter!=contacts.end(); iter++ )
	    {
            Communication::ContactInterface *contact = (*iter);
            contacts_map_[contact->GetID()] = contact;
        }

        friend_helper_->UpdateFriendList(contacts_map_);
    }

    void FriendListWidget::InitializeStatusEditor()
    {
        QStringList sorted_for_ui = QStringList();
        sorted_for_ui.append("available");
        sorted_for_ui.append("chat");
        sorted_for_ui.append("away");
        sorted_for_ui.append("xa");
        sorted_for_ui.append("dnd");
        sorted_for_ui.append("hidden");

        // Not yet, think the logic/ui first (this is a combo box and this item will be selected
        // when clicked, we need to get input and then set status back to previous index)
        //friend_list_ui_->statusEditor->addItem(QIcon(":images/iconRename.png"), "Set Status Message...");

        friend_list_ui_->statusEditor->clear();
        foreach (QString status_code, sorted_for_ui)
        {
            friend_list_ui_->statusEditor->addItem(UiDefines::PresenceStatus::GetIconForStatusCode(status_code),
                                                   UiDefines::PresenceStatus::GetStatusForStatusCode(status_code));
        }
    }

    void FriendListWidget::FriendClicked(QListWidgetItem *friend_item)
    {
        clicked_item_ = dynamic_cast<CommunicationUI::FriendListItem *>(friend_item);
        QMenu *friend_actions_menu = new QMenu(this);
        friend_actions_menu->addAction(QIcon(":images/iconChat.png"), "Start Chat", this, SLOT( StartChatSession() ));
        friend_actions_menu->addAction(QIcon(":images/iconVideo.png"), "Start Video Conversation", this, SLOT( StartVideoSession() ));
        friend_actions_menu->addSeparator();
        friend_actions_menu->addAction(QIcon(":images/iconRename.png"), "Rename"); // add slot
        friend_actions_menu->addAction(QIcon(":images/iconRemove.png"), "Remove"); // add slot
        friend_actions_menu->popup(QCursor::pos());
    }

    void FriendListWidget::SetStatus(const QString &status_code)
    {
        int index = friend_list_ui_->statusEditor->findText(UiDefines::PresenceStatus::GetStatusForStatusCode(status_code));
        if (index != -1)
            friend_list_ui_->statusEditor->setCurrentIndex(index);
    }

    void FriendListWidget::StatusChange(const QString &item_string)
    {
        if (item_string != "Set Status Message...")
            emit StatusChanged(UiDefines::PresenceStatus::GetStatusCodeForStatus(item_string));
    }

    void FriendListWidget::StartChatSession()
    {
        if (!session_helper_->DoesChatTabExist(clicked_item_->GetID()))
        {
            Communication::ContactInterface *contact = friend_helper_->GetContactsMap()[clicked_item_->GetID()];
            if (contact)
            {
                Communication::ChatSessionInterface *chat_session = im_connection_->OpenPrivateChatSession(*contact);
                QString id = contact->GetID();
                emit NewChatSessionStarted(chat_session, id);
            }
        }
    }

    void FriendListWidget::StartVideoSession()
    {
        Communication::ContactInterface *contact = friend_helper_->GetContactsMap()[clicked_item_->GetID()];
        if (contact)
        {
            Communication::VoiceSessionInterface *video_session = im_connection_->OpenVoiceSession(*contact);
            QString id = contact->GetID();
            emit NewVideoSessionStarted(video_session, contact->GetID());
        }
    }

}
