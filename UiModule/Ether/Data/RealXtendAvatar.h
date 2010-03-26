// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_RealXtendAvatar_h
#define incl_UiModule_RealXtendAvatar_h

#include "AvatarInfo.h"

#include <QUrl>

namespace Ether
{
    namespace Data
    {
        class RealXtendAvatar : public AvatarInfo
        {

        Q_OBJECT
        Q_PROPERTY(QString account_ READ account)
        Q_PROPERTY(QUrl auth_url_ READ authUrl)

        public:
            RealXtendAvatar(QString account, QUrl auth_url, QString password, QString path_to_pixmap = QString(""), QUuid id = 0)
                : AvatarInfo(Ether::AvatarTypes::RealXtend, password, path_to_pixmap, id),
                  account_(account),
                  auth_url_(auth_url)
            {
            }

            QString account()                   { return account_; }
            QUrl authUrl()                      { return auth_url_; }

            void setAccount(QString account)    { account_ = account; }
            void setAuthUrl(QUrl auth_url)      { auth_url_ = auth_url; }

            void Print()
            {
                qDebug() << "<RealXtendAvatar>";
                qDebug() << "    Type enum: " << avatarType();
                qDebug() << "    id: " << id();
                qDebug() << "    account: " << account_;
                qDebug() << "    auth url: " << auth_url_;
                qDebug() << "    secret: " << password();
                qDebug() << "    image path: " << pixmapPath();
            }

        private:
            QString account_;
            QUrl auth_url_;

        };
    }
}
#endif // REALXTENDAVATAR_H
