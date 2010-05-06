// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_WorldInfo_h
#define incl_UiModule_WorldInfo_h

#include <QObject>
#include <QString>
#include <QUuid>
#include <QUrl>
#include <QDebug>
#include <QVariant>
#include <QMap>

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
		Q_PROPERTY(QString start_location_ READ startLocation)

        public:
            WorldInfo(Ether::WorldTypes::World world_type, QUrl login_url, QString start_location, QMap<QString, QVariant> grid_info, QString path_to_pixmap, QUuid id = 0);

        public slots:
            virtual void Print() = 0;

            //! Getters
            QString id()                                 { return id_string_; }
            QString pixmapPath()                         { return path_to_pixmap_; }
            QUrl loginUrl()                              { return login_url_; }
			QString startLocation()						 { return start_location_; }
            QMap<QString, QVariant> gridInfo()           { return grid_info_; }
            Ether::WorldTypes::World worldType()         { return world_type_; }

            //! Setters
            void setPixmapPath(QString path)             { path_to_pixmap_ = path; }
            void setLoginUrl(QUrl login_url)             { login_url_ = login_url; }
			void setStartLocation(QString start_location){ start_location_ = start_location; }
            void setGridInfo(QMap<QString,QVariant> map) { grid_info_ = map; }

        private:
            QUuid id_;
            QString id_string_;
            QString path_to_pixmap_;
            Ether::WorldTypes::World world_type_;
            QUrl login_url_;
			QString start_location_;
            QMap<QString, QVariant> grid_info_;
        };
    }
}

#endif // WORLDINFO_H
