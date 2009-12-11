// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "FriendHelper.h"
#include "FriendListItem.h"

namespace UiHelpers
{
    FriendHelper::FriendHelper()
        : QObject(0),
          friend_list_ui_(0)
    {

    }

    FriendHelper::~FriendHelper()
    {
        friend_list_ui_ = 0;
    }

    void FriendHelper::NewFriendRequest(Communication::FriendRequestInterface &friend_request)
    {

    }

    void FriendHelper::OnNewContact(const Communication::ContactInterface &contact)
    {
        //contacts_map_[contact.GetID()] = (Communication::ContactInterface *)&contact;

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
                    friend_list_ui_->friendListWidget->removeItemWidget(list_item);
                    friend_list_ui_->friendListWidget->sortItems();
                    contacts_map_.remove(remove_id);
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