// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "FriendHelper.h"
#include "FriendListItem.h"
//#include "interface.h"
#include "ModuleManager.h"

#include "FriendRequestInterface.h"
#include "ContactInterface.h"
#include "CoreDefines.h"
#include "Framework.h"

#include <UiModule.h>
#include "Inworld/InworldSceneController.h"
#include "UiProxyWidget.h"

#include <QLabel>

#include "MemoryLeakCheck.h"

namespace UiHelpers
{
    FriendHelper::FriendHelper(Foundation::Framework *framework)
        : QObject(),
          friend_list_ui_(0),
          request_manager_widget_(0),
          framework_(framework)
    {

    }

    FriendHelper::~FriendHelper()
    {
        friend_list_ui_ = 0;
    }

    void FriendHelper::SetupUi(Ui::FrienListWidget *friend_list_ui)
    { 
        SAFE_DELETE(friend_list_ui_); 
        friend_list_ui_ = friend_list_ui;
    }

    void FriendHelper::NewFriendRequest(Communication::FriendRequestInterface &friend_request)
    {
        Communication::FriendRequestInterface *friend_request_ptr = dynamic_cast<Communication::FriendRequestInterface *>(&friend_request);
        if (friend_request_ptr)
        {
            pending_friend_requests_[friend_request.GetOriginatorID()] = friend_request_ptr;
            
            QString info_string;
            int pending_count = pending_friend_requests_.count();
            if (pending_count == 1)
                info_string = "new friend request";
            else
                info_string = "new friend requests";

            friend_list_ui_->pendingRequestsButton->setText(QString("%1 %2").arg(QString::number(pending_friend_requests_.count()), info_string));
            friend_list_ui_->friendRequestFrame->show();
        }
    }

    void FriendHelper::ShowRequestManagerWidget()
    {
        SAFE_DELETE(request_manager_widget_);
        request_manager_widget_ = new QWidget();
        request_manager_ui_.setupUi(request_manager_widget_);

        int row = 0;
        foreach (Communication::FriendRequestInterface *friend_request , pending_friend_requests_.values())
        {
            QLabel *id = new QLabel(friend_request->GetOriginatorID(), request_manager_widget_);
            QPushButton *accept_button = new QPushButton("Accept", request_manager_widget_);
            QPushButton *reject_button = new QPushButton("Reject", request_manager_widget_);

            request_manager_ui_.gridLayout->addWidget(id, row, 0);
            request_manager_ui_.gridLayout->addWidget(accept_button, row, 1);
            request_manager_ui_.gridLayout->addWidget(reject_button, row, 2);

            // For friend request
            connect(accept_button, SIGNAL( clicked() ), friend_request, SLOT( Accept() ));
            connect(reject_button, SIGNAL( clicked() ), friend_request, SLOT( Reject() ));
            // Hide all elements if clicked
            // Kind of ugly but works, otherwise we would need to keep a container for storing all the widgets with row count
            // so we could remove them from the layout in CheckPendingRequestList
            connect(accept_button, SIGNAL( clicked() ), id, SLOT( hide() ));
            connect(accept_button, SIGNAL( clicked() ), accept_button, SLOT( hide() ));
            connect(accept_button, SIGNAL( clicked() ), reject_button, SLOT( hide() ));
            connect(reject_button, SIGNAL( clicked() ), id, SLOT( hide() ));
            connect(reject_button, SIGNAL( clicked() ), accept_button, SLOT( hide() ));
            connect(reject_button, SIGNAL( clicked() ), reject_button, SLOT( hide() ));
            // For removing the request from list
            connect(accept_button, SIGNAL( clicked() ), this, SLOT( CheckPendingRequestList() ));
            connect(reject_button, SIGNAL( clicked() ), this, SLOT( CheckPendingRequestList() ));

            row++;
        }

        // Add friend request manager widget to scene
        UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
        if (ui_module)
        {
            request_manager_widget_->setWindowTitle(tr("Friend Requests"));
            UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(request_manager_widget_);
            proxy->show();
        }
    }

    void FriendHelper::CheckPendingRequestList()
    {
        foreach (Communication::FriendRequestInterface *friend_request , pending_friend_requests_.values())
        {
             Communication::FriendRequestInterface::State state = friend_request->GetState();
             switch (state)
             {
                case Communication::FriendRequestInterface::STATE_ACCEPTED:
                case Communication::FriendRequestInterface::STATE_REJECTED:
                {
                    pending_friend_requests_.remove(pending_friend_requests_.key(friend_request));
                    break;
                }
             }
        }

        QString info_string;
        int pending_count = pending_friend_requests_.count();
        if (pending_count == 1)
            info_string = "new friend request";
        else
            info_string = "new friend requests";

        friend_list_ui_->pendingRequestsButton->setText(QString("%1 %2").arg(QString::number(pending_friend_requests_.count()), info_string));
        friend_list_ui_->friendRequestFrame->show();

        if (pending_friend_requests_.empty())
        {
            request_manager_widget_->close();
            friend_list_ui_->friendRequestFrame->hide();
        }
    }

    void FriendHelper::OnNewContact(const Communication::ContactInterface &contact)
    {
        Communication::ContactInterface *contact_ptr = (Communication::ContactInterface *)(&contact);
        contacts_map_[contact.GetID()] = contact_ptr;

        CommunicationUI::FriendListItem *list_item = 
            new CommunicationUI::FriendListItem(friend_list_ui_->friendListWidget,
                                                contact.GetID(),
                                                contact.GetName(),
                                                contact.GetPresenceStatus(),
                                                contact.GetPresenceMessage());
        friend_list_ui_->friendListWidget->addItem(list_item);
        QObject::connect(&contact, SIGNAL( PresenceStatusChanged(const QString&, const QString&) ), list_item, SLOT( UpdateFriendsStatus(const QString&, const QString&) ));
        friend_list_ui_->friendListWidget->sortItems();
    }

    void FriendHelper::OnContactRemoved(const Communication::ContactInterface& contact)
    {
        QString remove_id = contact.GetID();

        for (int index = 0; index < friend_list_ui_->friendListWidget->count(); ++index)
        {
            CommunicationUI::FriendListItem *list_item = dynamic_cast<CommunicationUI::FriendListItem *>(friend_list_ui_->friendListWidget->item(index));
            if (list_item)
            {
                if (list_item->GetID() == remove_id)
                {
                    int row = friend_list_ui_->friendListWidget->row((QListWidgetItem *)list_item);
                    friend_list_ui_->friendListWidget->takeItem(row);
                    friend_list_ui_->friendListWidget->sortItems();
                    contacts_map_.remove(remove_id);
                    SAFE_DELETE(list_item);
                    return;
                }
            }
        }
    }

    void FriendHelper::UpdateFriendList(QMap<QString, Communication::ContactInterface*> contacts_map)
    {
        contacts_map_ = contacts_map;
        friend_list_ui_->friendListWidget->clear();

        foreach (Communication::ContactInterface *contact, contacts_map_.values())
        {
            CommunicationUI::FriendListItem *list_item = 
                new CommunicationUI::FriendListItem(friend_list_ui_->friendListWidget,
                                                    contact->GetID(),
                                                    contact->GetName(),
                                                    contact->GetPresenceStatus(),
                                                    contact->GetPresenceMessage());
            friend_list_ui_->friendListWidget->addItem(list_item);
            QObject::connect(contact, SIGNAL( PresenceStatusChanged(const QString&, const QString&) ), list_item, SLOT( UpdateFriendsStatus(const QString&, const QString&) ));
        }

        friend_list_ui_->friendListWidget->sortItems();
    }
}