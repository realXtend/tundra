// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DataManager.h"

#include "RealXtendAvatar.h"
#include "OpenSimAvatar.h"
#include "OpenSimWorld.h"

#include <QStringList>

namespace Ether
{
    namespace Data
    {
        DataManager::DataManager(QObject *parent)
            : QObject(parent),
              avatar_settings_name_("ether/avatarinfocards"),
              worldserver_settings_name_("ether/worldserverinfocards")
        {
        }
        
        int DataManager::GetAvatarCountInSettings()
        {
            QSettings avatar_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", avatar_settings_name_);
            return avatar_settings.childGroups().count();
        }

        int DataManager::GetWorldCountInSettings()
        {
            QSettings world_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", worldserver_settings_name_);
            return world_settings.childGroups().count();
        }

        QMap<QUuid, Data::AvatarInfo *> DataManager::ReadAllAvatarsFromFile()
        {
            QMap<QUuid, Data::AvatarInfo *> map;
            QSettings avatar_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", avatar_settings_name_);

            foreach (QString avatar_id, avatar_settings.childGroups())
            {
                QUuid id(avatar_id);
                if (!id.isNull())
                {
                    // Get all common values out of the config
                    int type = avatar_settings.value(QString("%1/type").arg(avatar_id)).toInt();
                    QByteArray secret = QByteArray::fromBase64(avatar_settings.value(QString("%1/secret").arg(avatar_id)).toByteArray());
                    QString image_path = avatar_settings.value(QString("%1/imagepath").arg(avatar_id)).toString();

                    // Umm.. default image test baby
                    if (image_path.isEmpty())
                        image_path = "./data/ui/images/ether/naali.jpg";

                    switch (type)
                    {
                        case AvatarTypes::RealXtend:
                        {

                            Data::RealXtendAvatar *realxtend_avatar =
                                    new Data::RealXtendAvatar(avatar_settings.value(QString("%1/account").arg(avatar_id)).toString(),
                                                              avatar_settings.value(QString("%1/authurl").arg(avatar_id)).toUrl(),
                                                              secret,
                                                              image_path,
                                                              id);
                            map[id] = realxtend_avatar;
                            break;
                        }

                        case AvatarTypes::OpenSim:
                        {
                            Data::OpenSimAvatar *opensim_avatar =
                                    new Data::OpenSimAvatar(avatar_settings.value(QString("%1/firstname").arg(avatar_id)).toString(),
                                                            avatar_settings.value(QString("%1/lastname").arg(avatar_id)).toString(),
                                                            secret,
                                                            image_path,
                                                            id);
                            map[id] = opensim_avatar;
                            break;
                        }

                        case AvatarTypes::OpenID:
                            qDebug() << "Can't store OpenID type yet, so can't read them either";
                            break;

                        default:
                            qDebug() << "File had unrecognized avatar type, this should never happen!";
                    }
                }
            }               
            return map;
        }

        void DataManager::StoreOrUpdateAvatar(Data::AvatarInfo *avatar_info)
        {
            QSettings avatar_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", avatar_settings_name_);
            QString uuid_string = avatar_info->id();
            AvatarTypes::Avatar type = avatar_info->avatarType();

            if (!avatar_settings.childGroups().contains(uuid_string))
            {
                avatar_settings.beginGroup(uuid_string);
                avatar_settings.setValue("type", avatar_info->avatarType());

                qDebug() << "Stored the following avatar to local file";
                switch (type)
                {
                    case AvatarTypes::RealXtend:
                    {
                        Data::RealXtendAvatar *realxtend_avatar = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                        if (realxtend_avatar)
                        {
                            avatar_settings.setValue("account", realxtend_avatar->account());
                            avatar_settings.setValue("authurl", realxtend_avatar->authUrl());
                            avatar_settings.setValue("secret", QByteArray(realxtend_avatar->password().toStdString().c_str()).toBase64());
                            avatar_settings.setValue("imagepath", realxtend_avatar->pixmapPath());
                            realxtend_avatar->Print();
                        }
                        else
                            qDebug() << "Could not cast AvatarInfo to RealXtendAvatar";
                        break;
                    }

                    case AvatarTypes::OpenSim:
                    {
                        Data::OpenSimAvatar *opensim_avatar = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                        if (opensim_avatar)
                        {
                            avatar_settings.setValue("firstname", opensim_avatar->firstName());
                            avatar_settings.setValue("lastname", opensim_avatar->lastName());
                            avatar_settings.setValue("secret", QByteArray(opensim_avatar->password().toStdString().c_str()).toBase64());
                            avatar_settings.setValue("imagepath", opensim_avatar->pixmapPath());
                            opensim_avatar->Print();
                        }
                        else
                            qDebug() << "Could not cast AvatarInfo to OpenSimAvatar";
                        break;
                    }

                    case AvatarTypes::OpenID:
                    case AccountTypes::None:
                        qDebug() << "OpenID and None typed avatars cannot be stored to file yet...";
                        break;

                    default:
                        qDebug() << "Tried to store a avatar with invalid type";
                        break;
                }
                avatar_settings.endGroup();
                avatar_settings.sync();
                qDebug() << endl;
            }
            else
            {
                qDebug() << "Updated the following avatars info to local file";
                switch (type)
                {
                    case AvatarTypes::RealXtend:
                    {
                        Data::RealXtendAvatar *realxtend_avatar = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                        if (realxtend_avatar)
                        {
                            avatar_settings.setValue(QString("%1/account").arg(uuid_string), realxtend_avatar->account());
                            avatar_settings.setValue(QString("%1/authurl").arg(uuid_string), realxtend_avatar->authUrl());
                            avatar_settings.setValue(QString("%1/secret").arg(uuid_string), QByteArray(realxtend_avatar->password().toStdString().c_str()).toBase64());
                            avatar_settings.setValue(QString("%1/imagepath").arg(uuid_string), realxtend_avatar->pixmapPath());
                            realxtend_avatar->Print();
                        }
                        else
                            qDebug() << "Could not cast AvatarInfo to RealXtendAvatar";
                        break;
                    }

                    case AvatarTypes::OpenSim:
                    {
                        Data::OpenSimAvatar *opensim_avatar = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                        if (opensim_avatar)
                        {
                            avatar_settings.setValue(QString("%1/firstname").arg(uuid_string), opensim_avatar->firstName());
                            avatar_settings.setValue(QString("%1/lastname").arg(uuid_string), opensim_avatar->lastName());
                            avatar_settings.setValue(QString("%1/secret").arg(uuid_string), QByteArray(opensim_avatar->password().toStdString().c_str()).toBase64());
                            avatar_settings.setValue(QString("%1/imagepath").arg(uuid_string), opensim_avatar->pixmapPath());
                            opensim_avatar->Print();
                        }
                        else
                            qDebug() << "Could not cast AvatarInfo to OpenSimAvatar";

                        break;
                    }

                    case AvatarTypes::OpenID:
                    case AccountTypes::None:
                        qDebug() << "OpenID and None typed avatars cannot be updated to file yet...";
                        break;

                    default:
                        qDebug() << "Tried to update a avatar with invalid type";
                        break;
                }
                avatar_settings.sync();
            }
        }

        QMap<QUuid, Data::WorldInfo*> DataManager::ReadAllWorldsFromFile()
        {
            QMap<QUuid, Data::WorldInfo *> map;
            QSettings world_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", worldserver_settings_name_);

            foreach (QString world_id, world_settings.childGroups())
            {
                QUuid id(world_id);
                if (!id.isNull())
                {
                    // Get all common values out of the config
                    int type = world_settings.value(QString("%1/type").arg(world_id)).toInt();
                    QString image_path = world_settings.value(QString("%1/imagepath").arg(world_id)).toString();
                    QUrl login_url = world_settings.value(QString("%1/loginurl").arg(world_id)).toUrl();
                    QUrl info_url = world_settings.value(QString("%1/infourl").arg(world_id)).toUrl();

                    // Umm.. default image test baby
                    if (image_path.isEmpty())
                        image_path = "./data/ui/images/ether/world.jpg";

                    switch (type)
                    {
                        case WorldTypes::OpenSim:
                        {
                            Data::OpenSimWorld *opensim_world = new Data::OpenSimWorld(login_url,
                                                                                       info_url,
                                                                                       image_path,
                                                                                       id);
                            map[id] = opensim_world;
                            break;
                        }

                        case WorldTypes::Taiga:
                        case WorldTypes::None:
                            qDebug() << "Can't store Taiga or None types yet, so can't read them either";
                            break;

                        default:
                            qDebug() << "File had unrecognized avatar type, this should never happen!";
                            break;
                    }
                }
            }
            return map;
        }

        void DataManager::StoreOrUpdateWorld(Data::WorldInfo *world_info)
        {
            QSettings world_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", worldserver_settings_name_);
            QString uuid_string = world_info->id();
            WorldTypes::World type = world_info->worldType();

            if (!world_settings.childGroups().contains(uuid_string))
            {
                world_settings.beginGroup(uuid_string);
                world_settings.setValue("type", world_info->worldType());

                qDebug() << "Stored the following world to local file";
                switch (type)
                {
                    case WorldTypes::OpenSim:
                    {
                        Data::OpenSimWorld *opensim_world = dynamic_cast<Data::OpenSimWorld *>(world_info);
                        if (opensim_world)
                        {
                            world_settings.setValue("loginurl", opensim_world->loginUrl());
                            world_settings.setValue("infourl", opensim_world->infoUrl());
                            world_settings.setValue("imagepath", opensim_world->pixmapPath());
                            opensim_world->Print();
                        }
                        else
                            qDebug() << "Could not cast WorldInfo to OpenSimWorld";
                        break;
                    }

                    case WorldTypes::Taiga:
                    case WorldTypes::None:
                        qDebug() << "Taiga and None typed world cannot be stored to file yet...";;
                        break;

                    default:
                        qDebug() << "Tried to store a avatar with invalid type";
                        break;
                }
                world_settings.endGroup();
                world_settings.sync();
                qDebug() << endl;
            }
            else
            {
                qDebug() << "Updated the following worlds info to local file";
                switch (type)
                {
                    case WorldTypes::OpenSim:
                    {
                        Data::OpenSimWorld *opensim_world = dynamic_cast<Data::OpenSimWorld *>(world_info);
                        if (opensim_world)
                        {
                            world_settings.setValue(QString("%1/loginurl").arg(uuid_string), opensim_world->loginUrl());
                            world_settings.setValue(QString("%1/infourl").arg(uuid_string), opensim_world->infoUrl());
                            world_settings.setValue(QString("%1/imagepath").arg(uuid_string), opensim_world->pixmapPath());
                        }
                        else
                            qDebug() << "Could not cast WorldInfo to OpenSimWorld";
                        break;
                    }

                    case WorldTypes::Taiga:
                    case WorldTypes::None:
                        qDebug() << "Taiga and None typed world cannot be updated to file yet...";;
                        break;

                    default:
                        qDebug() << "Tried to update a avatar with invalid type";
                        break;
                }
                world_settings.sync();
            }
        }
    }
}
