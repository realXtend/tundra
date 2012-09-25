#ifndef incl_XMPP_UserItem_h
#define incl_XMPP_UserItem_h

#include "Framework.h"

#include <qxmpp/QXmppRosterIq.h>

#include <QObject>
#include <QMap>
#include <QByteArray>

class QXmppVCardIq;
class QXmppPresence;

namespace XMPP
{

//! Represents single entry in user's roster
//!
//!
//! This class is contained within XMPP:Client don't use directly
class UserItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList resources READ GetResources);
    Q_PROPERTY(QString jid READ Jid);
    Q_PROPERTY(QString birthday READ GetBirthday);
    Q_PROPERTY(QString email READ GetEmail);
    Q_PROPERTY(QString fullname READ GetFullName);
    Q_PROPERTY(QString url READ GetUrl);

public:
    struct ResourceItem
    {
        bool available;
        QStringList capabilities; // QStringList for script friendliness (scripts don't mix well with enums)
    };

    UserItem(const QString &bareJid);
    ~UserItem();

    void UpdateRosterItem(const QXmppRosterIq::Item &item);
    void UpdatePresence(const QString &resource, const QXmppPresence &presence);
    void UpdateVCard(const QXmppVCardIq &vcard);

public slots:
    //! Get user's Jabber ID
    QString Jid() { return bare_jid_; }

    //! Get all the capabilities for user/resource
    //! \param resource capabilities of the resource, if empty
    //!        all the capabilities the user has will be returned
    //! \return QStringList containing resource/user capabilities,
    //!         empty if resource not found.
    QStringList GetCapabilities(QString resource = QString());

    //! Get user's resources
    QStringList GetResources();

    //! Get status of the user's vCard
    bool HasVCard() { return has_vcard_; }

    //! Number of clients connected to user's Jabber ID
    int ResourceCount() { return resources_.size(); }

    //! Returns users availability status
    //! \return true if atleast one of the user's resources is available
    bool IsAvailable() const { return available_; }

    QString GetBirthday() { return birthday_; }
    QString GetEmail() { return email_; }
    QString GetFullName() { return full_name_; }
    QByteArray GetPhoto() { return photo_; }
    QString GetPhotoType() { return photo_type_; }
    QString GetUrl() { return url_; }

private:
    //! Check if any of the user's resources are in available state
    void CheckAvailability();

    QMap<QString, ResourceItem> resources_;
    QString bare_jid_;

    QString birthday_;
    QString email_;
    QString full_name_;
    QByteArray photo_;
    QString photo_type_;
    QString url_;

    bool available_;
    bool has_vcard_;

signals:
    void AvailabilityChanged(bool availability);
};

} // end of namespace: XMPP

#endif // incl_XMPP_UserItem_h
