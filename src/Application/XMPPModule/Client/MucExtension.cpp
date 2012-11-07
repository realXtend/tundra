// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MucExtension.h"
#include "Client.h"
#include "XMPPModule.h"

#include "LoggingFunctions.h"
#include "Framework.h"

#include "qxmpp/QXmppMucManager.h"
#include "qxmpp/QXmppUtils.h"
#include "qxmpp/QXmppMessage.h"
#include "qxmpp/QXmppPresence.h"

#include "MemoryLeakCheck.h"

namespace XMPP
{

QString MucExtension::extension_name_ = "Muc";

MucExtension::MucExtension() :
    Extension(extension_name_),
    qxmpp_muc_manager_(new QXmppMucManager())
{
}

MucExtension::~MucExtension()
{
    foreach(QXmppMucRoom *room, rooms_)
    {
        room->leave("Logging off!");
    }
}

void MucExtension::Initialize(Client *client)
{
    client_ = client;
    framework_ = client_->GetFramework();

    client_->GetQxmppClient()->addExtension(qxmpp_muc_manager_);

    bool check = connect(qxmpp_muc_manager_, SIGNAL(invitationReceived(QString,QString,QString)), this, SLOT(HandleInvitationReceived(QString,QString,QString)));
    Q_ASSERT(check);
}

void MucExtension::HandleMessageReceived(const QXmppMessage &message)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    QString message_type;

    // Parse message type into string.
    switch(message.type())
    {
        case QXmppMessage::Error:
            message_type = "error";
            break;
        case QXmppMessage::Normal:
            message_type = "normal";
            break;
        case QXmppMessage::Chat:
            message_type = "chat";
            break;
        case QXmppMessage::GroupChat:
            message_type = "groupchat";
            break;
        case QXmppMessage::Headline:
            message_type = "headline";
            break;
        default:
            message_type = "invalid type";
    }

    LogInfo("XMPPModule: Received message. From: " + message.from().toStdString()
            + " Room: " + room->jid().toStdString()
            + " Body: " + message.body().toStdString()
            + " Type: " + message_type.toStdString());

    emit MessageReceived(room->jid(), message.from(), message.body(), message_type);
}

void MucExtension::HandleInvitationReceived(const QString &room, const QString &inviter, const QString &reason)
{
    LogDebug(extension_name_.toStdString() + ": Received invitation (room ="
                         + room.toStdString() + " inviter ="
                         + inviter.toStdString() + " reason = "
                         + reason.toStdString() +")");
    emit InvitationReceived(room, inviter, reason);
}



/*void MucExtension::handlePresenceReceived(const QXmppPresence &presence)
{
    QString from_domain = jidToDomain(presence.from());
    if(!from_domain.contains("conference"))
        return;

    QString room_jid = jidToBareJid(presence.from());
    QString nickname = jidToResource(presence.from());

    if(rooms_.keys().contains(room_jid))
        rooms_[room_jid]->setNickname(nickname);
    else
        handleRoomAdded(room_jid, nickname);
}*/

void MucExtension::HandleParticipantJoined(const QString &jid)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    LogDebug(extension_name_.toStdString() + ": User joined. Room: " + room->jid().toStdString()
                        + " User: " + jid.toStdString());

    emit UserJoinedRoom(room->jid(), jid);
}

void MucExtension::HandleParticipantLeft(const QString &jid)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    LogDebug(extension_name_.toStdString() + ": User left. Room: " + room->jid().toStdString()
                        + " User: " + jid.toStdString());

    emit UserLeftRoom(room->jid(), jid);
}

void MucExtension::HandleRoomJoined()
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    bool check = connect(room, SIGNAL(messageReceived(QXmppMessage)), this, SLOT(HandleMessageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(room, SIGNAL(participantAdded(QString)), this, SLOT(HandleParticipantJoined(QString)));
    Q_ASSERT(check);

    check = connect(room, SIGNAL(participantRemoved(QString)), this, SLOT(HandleParticipantLeft(QString)));
    Q_ASSERT(check);

    check = connect(room, SIGNAL(kicked(QString,QString)), this, SIGNAL(RoomRemoved(QString,QString)));
    Q_ASSERT(check);

    rooms_.append(room);
    emit RoomAdded(room->jid(), room->nickName());
}

void MucExtension::HandleRoomSubjectChanged(const QString &subject)
{
    QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
    if(!room)
        return;

    emit RoomSubjectChanged(room->jid(), subject);
}

bool MucExtension::JoinRoom(QString roomJid, QString nickname, QString password)
{
    QXmppMucRoom *room = qxmpp_muc_manager_->addRoom(roomJid);
    room->setNickName(nickname);
    room->setPassword(password);

    bool check = connect(room, SIGNAL(joined()), this, SLOT(HandleRoomJoined()));
    Q_ASSERT(check);

    return room->join();
}

bool MucExtension::LeaveRoom(QString roomJid)
{
    QXmppMucRoom *room = GetRoom(roomJid);
    if(!room)
        return false;

    return room->leave("Bye!");
}

bool MucExtension::SendMessage(QString roomJid, QString message)
{
    QXmppMucRoom *room = GetRoom(roomJid);
    if(!room)
        return false;

    return room->sendMessage(message);
}

QString MucExtension::GetRoomSubject(QString roomJid)
{
    QXmppMucRoom *room = GetRoom(roomJid);
    if(!room)
        return QString("unknown");

    return room->subject();
}

bool MucExtension::Invite(QString roomJid, QString peerJid, QString reason)
{
    QXmppMucRoom *room = GetRoom(roomJid);
    if(!room)
        return false;

    return room->sendInvitation(peerJid, reason);
}

QStringList MucExtension::GetParticipants(QString roomJid)
{
    QXmppMucRoom *room = GetRoom(roomJid);
    if(!room)
        return QStringList();

    return room->participants();
}

QStringList MucExtension::GetRooms() const
{
    QStringList rooms;
    foreach(QXmppMucRoom *room, rooms_)
    {
        rooms.append(room->jid());
    }
    return rooms;
}

QXmppMucRoom* MucExtension::GetRoom(const QString &roomJid)
{
    foreach(QXmppMucRoom *room, rooms_)
    {
        if(room->jid() == roomJid)
            return room;
    }
    LogError("XMPPModule: No such room: " + roomJid.toStdString());
    return 0;
}



} // end of namespace: XMPP
