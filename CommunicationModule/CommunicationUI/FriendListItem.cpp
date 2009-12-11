// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "FriendListItem.h"
#include "UiDefines.h"

namespace CommunicationUI
{
    FriendListItem::FriendListItem(QListWidget *parent, QString &id, QString &name, QString &status, QString &status_message)
        : QObject(),
          QListWidgetItem(parent),
          id_(id),
          name_(name),
          current_status_(status),
          current_status_message_(status_message)
    {
        setIcon(UiDefines::PresenceStatus::GetIconForStatusCode(GetStatus()));
        if (GetStatusMessage().isEmpty())
            setText(name_);
        else
            setText(name_.append("\n%1").arg(GetStatusMessage()));
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
            SetStatusMessage(new_message);
            if (GetStatusMessage().isEmpty())
                setText(name_);
            else
                setText(name_.append("\n%1").arg(GetStatusMessage()));
        }
    }

}