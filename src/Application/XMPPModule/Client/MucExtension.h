/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MucExtension.h
 *  @brief  Extension for XMPP:Client, provides multi-user chatroom support (XEP-0045).
 */

#ifndef incl_XMPP_MucExtension_h
#define incl_XMPP_MucExtension_h

#include "Extension.h"

#include <QObject>
#include <QString>
#include <QList>

class QXmppMucManager;
class QXmppMucRoom;
class QXmppMessage;
class QXmppPresence;

class Framework;

namespace XMPP
{
class Client;

//! Handles multiuser chat messaging as defined in XEP-0045
class MucExtension : public Extension
{
    Q_OBJECT
    Q_PROPERTY(QStringList rooms READ GetRooms);

public:
    MucExtension();
    virtual ~MucExtension();
    virtual void Initialize(Client *client);

public slots:
    //! Join multiuser chatroom on the server
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param nickname Nickname to be used in the room
    //! \param password Optional password for the room
    //! \return bool true for succesful join request
    //! \note Succesful join request does not mean the actual join was succesful
    bool JoinRoom(QString roomJid, QString nickname, QString password = QString());

    //! Leave muc chatroom
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param bool true for room found and left
    bool LeaveRoom(QString roomJid);

    //! Send message to muc chatroom
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \param message Message to sent
    //! \return bool true for succesfully sent message
    bool SendMessage(QString roomJid, QString message);

    //! Get current subject for a room
    //! \param room Full JabberID for the room (room@conference.host.com)
    //! \return QString room subject
    QString GetRoomSubject(QString roomJid);

    //! Get list of currently active rooms
    //! \return QStringList containing full JabberIDs of the rooms
    QStringList GetRooms() const;

    //! Get participants for given room
    //! \param room Full JabberID for the room
    //! \return QStringList containing participant nicknames for the room
    QStringList GetParticipants(QString roomJid);

    //! Invite user to chatroom
    //! \param room Room the user is invited to
    //! \param peerJid JabberID of the remote user
    //! \param reason Optional reason message
    //! \return bool true for succesful invite
    bool Invite(QString roomJid, QString peerJid, QString reason = QString());

private slots:
    void HandleMessageReceived(const QXmppMessage &message);
    void HandleInvitationReceived(const QString &room, const QString &inviter, const QString &reason);
    void HandleParticipantJoined(const QString &jid);
    void HandleParticipantLeft(const QString &jid);
    void HandleRoomSubjectChanged(const QString &subject);
    void HandleRoomJoined();

private:
    static QString extension_name_;
    QXmppMucManager* qxmpp_muc_manager_;
    //QMap<QString, MucRoom*> rooms_;
    QList<QXmppMucRoom*> rooms_;
    Framework *framework_;
    Client *client_;

protected:
    QXmppMucRoom *GetRoom(const QString &roomJid);

signals:
    void MessageReceived(QString room, QString sender, QString message, QString type);
    void InvitationReceived(QString room, QString from, QString reason);
    void RoomAdded(QString room, QString nickname);
    void RoomRemoved(QString room, QString reason);
    void RoomSubjectChanged(QString room, QString subject);
    void UserJoinedRoom(QString room, QString user);
    void UserLeftRoom(QString room, QString user);
};

} // end of namespace: XMPP

#endif // incl_XMPP_MucExtension_h
