// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "Math/float3.h"

#include <QObject>
#include <QString>

class MumbleUser : public QObject
{
Q_OBJECT

Q_PROPERTY(uint id READ Id)
Q_PROPERTY(uint channelId READ ChannelId)
Q_PROPERTY(QString name READ Name)
Q_PROPERTY(QString comment READ Comment)
Q_PROPERTY(QString hash READ Hash)
Q_PROPERTY(bool isMuted READ IsMuted)
Q_PROPERTY(bool isSelfMuted READ IsSelfMuted)
Q_PROPERTY(bool isSelfDeaf READ IsSelfDeaf)
Q_PROPERTY(bool isMe READ IsMe)

public:
    MumbleUser();
    ~MumbleUser();

    uint id;
    uint channelId;
    QString name;
    QString comment;
    QString hash;
    bool isMuted;
    bool isSelfMuted;
    bool isSelfDeaf;
    bool isMe;
    bool isPositional;
    float3 pos;

    uint Id() { return id; }
    uint ChannelId() { return channelId; }
    QString Name() { return name; }
    QString Comment() { return comment; }
    QString Hash() { return hash; }
    bool IsMuted() { return isMuted; }
    bool IsSelfMuted() { return isSelfMuted; }
    bool IsSelfDeaf() { return isSelfDeaf; }
    bool IsMe() { return isMe; }

public slots:
    QString toString() const;
};

class MumbleChannel : public QObject
{
Q_OBJECT

Q_PROPERTY(uint id READ Id)
Q_PROPERTY(uint parentId READ ParentId)
Q_PROPERTY(QString name READ Name)
Q_PROPERTY(QString fullName READ FullName)
Q_PROPERTY(QString description READ Description)
Q_PROPERTY(QList<MumbleUser*> users READ Users)

public:
    MumbleChannel();
    ~MumbleChannel();

    uint id;
    uint parentId;
    QString name;
    QString fullName;
    QString description; 
    QList<MumbleUser*> users;

    uint Id() { return id; }
    uint ParentId() { return parentId; }
    QString Name() { return name; }
    QString FullName() { return fullName; }
    QString Description() { return description; }
    QList<MumbleUser*> Users() { return users; }

    /// @note Frees the user ptr.
    void RemoveUser(uint id); 
    void EmitUsersChanged() { emit UsersChanged(users); }

public slots:
    MumbleUser *User(uint id);
    QList<uint> MutedUserIds();

    QString toString() const;

signals:
    void UsersChanged(QList<MumbleUser*> users);
};
