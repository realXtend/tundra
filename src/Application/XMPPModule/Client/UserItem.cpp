// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UserItem.h"
#include "XMPPModule.h"

#include "LoggingFunctions.h"
#include "CoreStringUtils.h"

#include <qxmpp/QXmppUtils.h>
#include <qxmpp/QXmppPresence.h>
#include <qxmpp/QXmppVCardIq.h>

#include "MemoryLeakCheck.h"

namespace XMPP
{
    UserItem::UserItem(const QString &bareJid) :
        has_vcard_(false),
        available_(false)
    {
        bare_jid_ = bareJid;
    }

    UserItem::~UserItem()
    {

    }

    void UserItem::CheckAvailability()
    {
        bool available = false;

        if(resources_.size() == 0)
        {
            available_ = false;
            LogDebug("XMPPModule: " + bare_jid_.toStdString() + " changed availability to: " + (available_ ? "available" : "unavailable"));
            emit AvailabilityChanged(available_);
            return;
        }

        QString resource;
        foreach(resource, resources_.keys())
        {
            if(resources_[resource].available)
                available = true;
        }

        if(available_ != available)
        {
            available_ = available;
            LogDebug("XMPPModule: " + bare_jid_.toStdString() + " changed availability to: " + (available_ ? "available" : "unavailable"));
            emit AvailabilityChanged(available_);
        }
    }

    void UserItem::UpdatePresence(const QString &resource, const QXmppPresence &presence)
    {
        if(presence.type() == QXmppPresence::Available)
        {
            if(resources_.contains(resource))
                resources_.remove(resource);

            ResourceItem resource_item;
            resource_item.available = true;

            QStringList capabilities = presence.capabilityExt();
            QString capability;
            foreach(capability, capabilities)
                resource_item.capabilities.append(capability);

            resources_.insert(resource, resource_item);
            CheckAvailability();
        }
        else if(presence.type() == QXmppPresence::Unavailable)
        {
            if(resources_.contains(resource))
                resources_.remove(resource);
            CheckAvailability();
        }
    }

    void UserItem::UpdateRosterItem(const QXmppRosterIq::Item &item)
    {
        /// \note do we really need this at all?
    }

    void UserItem::UpdateVCard(const QXmppVCardIq &vcard)
    {
        birthday_ = vcard.birthday().toString("dd.MM.yyyy");
        email_ = vcard.email();
        full_name_ = vcard.fullName();
        photo_ = vcard.photo();
        url_ = vcard.url();
    }

    QStringList UserItem::GetResources()
    {
        return resources_.keys();
    }

    QStringList UserItem::GetCapabilities(QString resource)
    {
        QStringList capabilities;

        if(resource.isEmpty())
        {
            QString resource;
            foreach(resource, resources_.keys())
            {
                QString capability;
                foreach(capability, resources_[resource].capabilities)
                {
                    if(!capabilities.contains(capability))
                        capabilities.append(capability);
                }
            }
        }
        else
        {
            if(resources_.keys().contains(resource))
                capabilities = resources_[resource].capabilities;
        }

        return capabilities;
    }
} // end of namespace: XMPP

