// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_OpenSimAvatar_h
#define incl_UiModule_OpenSimAvatar_h

#include "AvatarInfo.h"

namespace Ether
{
    namespace Data
    {
        class OpenSimAvatar : public AvatarInfo
        {

        Q_OBJECT
        Q_PROPERTY(QString first_name_ READ firstName)
        Q_PROPERTY(QString last_name_ READ lastName)

        public:
            OpenSimAvatar(QString first_name, QString last_name, QString password, QString path_to_pixmap = QString(""), QUuid id = 0)
                : AvatarInfo(Ether::AvatarTypes::OpenSim, password, path_to_pixmap, id),
                  first_name_(first_name),
                  last_name_(last_name)
            {
            }

            QString firstName()                     { return first_name_; }
            QString lastName()                      { return last_name_; }
            QString userName()                      { return QString("%1 %2").arg(first_name_, last_name_); }

            void setFirstName(QString first_name)   { first_name_ = first_name; }
            void setLastName(QString last_name)     { last_name_ = last_name; }

            void Print()
            {
                qDebug() << "<OpenSimAvatar>";
                qDebug() << "    Type enum: " << avatarType();
                qDebug() << "    id: " << id();
                qDebug() << "    first name: " << first_name_;
                qDebug() << "    last name: " << last_name_;
                qDebug() << "    secret: " << password();
                qDebug() << "    image path: " << pixmapPath();
            }

        private:
            QString first_name_;
            QString last_name_;

        };
    }
}

#endif // OPENSIMAVATAR_H
