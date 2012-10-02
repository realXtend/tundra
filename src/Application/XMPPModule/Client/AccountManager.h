/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AccountManager.h
 *  @brief  Provides interface for storing server-username-password combinations to local storage.
 */

#ifndef incl_XMPP_AccountManager_h
#define incl_XMPP_AccountManager_h

#include <QObject>
#include <QDomElement>
#include <QByteArray>
#include <QStringList>

class Framework;

namespace XMPP
{

//! Handles saving/loading of user's login credentials
class AccountManager : public QObject
{
    Q_OBJECT

public:
    AccountManager(Framework* framework);
    ~AccountManager();

    void LoadFromFile();
    void SaveToFile();

public slots:
    //! Save login data
    //! \param bareJid User Jabber ID
    //! \param xmppServer Server the Jabber ID is associated with
    //! \param password User password
    //! \note IMPORTANT: Storing passwords uses weak Xor cipher. This is
    //!       not secure, and should be made very clear for the user.
    void AddLoginData(QString userJid, QString xmppServer, QString password);

    //! Get list of XMPP Servers we have stored credentials for
    //! \return QStringList containing known servers
    QStringList GetServers();

    //! Get saved Jabber IDs
    //! \param xmppServer Server associated with the Jid
    //! \return QStringList containing saved Jids
    QStringList GetUserJids(QString xmppServer);

    //! Get password for Jabber ID
    //! \param userJid Jabber ID for querying the password
    //! \param xmppServer XMPP Server the Jid is associated with
    //! \return QString containing the user password
    QString GetUserPassword(QString userJid, QString xmppServer);

private slots:

private:
    //! Generates Xor cipher for storing passwords in a file
    //! \note Protects from the casual idiot browsing through user's settings
    //!       folder, but in no way secure. Need to implement tundra wide
    //!       passwords storing policy.
    QByteArray GenerateXor(const QByteArray& data, const QByteArray& key);

    QString accounts_path_;
    QString accounts_filename_;
    QDomDocument *accounts_;
};

} // end of namespace: XMPP

#endif // incl_XMPP_SettingsManager_h
