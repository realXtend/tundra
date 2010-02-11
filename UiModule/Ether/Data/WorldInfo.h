// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_WorldInfo_h
#define incl_UiModule_WorldInfo_h

#include <QObject>
#include <QString>
#include <QUuid>
#include <QUrl>
#include <QDebug>

#include "EtherDataTypes.h"

namespace Ether
{
    namespace Data
    {
        class WorldInfo : public QObject
        {

        Q_OBJECT
        Q_PROPERTY(QString id_string_ READ id)
        Q_PROPERTY(QString path_to_pixmap_ READ pixmapPath WRITE setPixmapPath)
        Q_PROPERTY(QUrl login_url_ READ loginUrl)
        Q_PROPERTY(QUrl info_url_ READ infoUrl)

        public:
            WorldInfo(Ether::WorldTypes::World world_type, QUrl login_url, QUrl info_url, QString path_to_pixmap, QUuid id = 0);

        public slots:
            virtual void Print() = 0;

            //! Getters
            QString id() { return id_string_; }
            QString pixmapPath() { return path_to_pixmap_; }
            QUrl loginUrl() { return login_url_; }
            QUrl infoUrl() { return info_url_; }
            Ether::WorldTypes::World worldType() { return world_type_; }

            //! Setters
            void setPixmapPath(QString path) { path_to_pixmap_ = path; }

        private:
            QUuid id_;
            QString id_string_;
            QString path_to_pixmap_;
            Ether::WorldTypes::World world_type_;
            QUrl login_url_;
            QUrl info_url_;
        };
    }
}

#endif // WORLDINFO_H
