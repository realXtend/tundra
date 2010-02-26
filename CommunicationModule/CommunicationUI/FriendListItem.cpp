// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "FriendListItem.h"
#include "UiDefines.h"

#include "MemoryLeakCheck.h"

namespace CommunicationUI
{
    FriendListItem::FriendListItem(QListWidget *parent_widget, QString id, QString name, QString status, QString status_message)
        : QObject(),
          QListWidgetItem(parent_widget),
          id_(id),
          name_(name),
          current_status_(status),
          current_status_message_(status_message)
    {
        setIcon(UiDefines::PresenceStatus::GetIconForStatusCode(GetStatus()));

        QString status_string;
        if (current_status_message_.isEmpty() || current_status_message_.isNull())
            status_string = name_;
        else
            status_string = name_ + "\n" + current_status_message_;
        setText(status_string);
    }

    FriendListItem::~FriendListItem()
    {

    }

    void FriendListItem::UpdateFriendsStatus(const QString &new_status, const QString &new_message)
    {
        if (new_status != GetStatus())
        {
            SetStatus(new_status);
            setIcon(UiDefines::PresenceStatus::GetIconForStatusCode(GetStatus()));
        }

        if (new_message != GetStatusMessage())
        {
            QString status_string;
            if (new_message.isEmpty() || new_message.isNull())
                status_string = name_;
            else
                status_string = name_ + "\n" + new_message;
            
            SetStatusMessage(new_message);
            setText(status_string);
        }
    }

}