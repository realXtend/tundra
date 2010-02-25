// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_AvatarInfo_h
#define incl_UiModule_AvatarInfo_h

#include <QObject>
#include <QString>
#include <QUuid>
#include <QDebug>

#include "EtherDataTypes.h"

namespace Ether
{
    namespace Data
    {
        class AvatarInfo : public QObject
        {
            Q_OBJECT
            Q_PROPERTY(QString id_string_ READ id)
            Q_PROPERTY(QString password_ READ password)
            Q_PROPERTY(QString path_to_pixmap_ READ pixmapPath WRITE setPixmapPath)

        public:
            AvatarInfo(Ether::AvatarTypes::Avatar avatar_type, QString password, QString path_to_pixmap, QUuid id);

        public slots:
            virtual void Print() = 0;

            //! Getters
            QString id() const { return id_string_; }
            QString password() const { return password_; }
            QString pixmapPath() const { return path_to_pixmap_; }
            Ether::AvatarTypes::Avatar avatarType() const { return avatar_type_; }

            //! Setters
            void setPixmapPath(QString path) { path_to_pixmap_ = path; }
            void setPassword(QString password) { password_ = password; }

        private:
            QUuid id_;
            QString id_string_;
            QString password_;
            QString path_to_pixmap_;
            Ether::AvatarTypes::Avatar avatar_type_;
        };
    }
}
#endif // AVATARINFO_H
