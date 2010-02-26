// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "GridInfoHelper.h"

#include <QXmlStreamReader>
#include <QDebug>

namespace ProtocolUtilities
{
    GridInfoHelper::GridInfoHelper(QObject *parent, QUrl url)
        : QObject(parent),
          url_(url)
    {
        url_.setPath("get_grid_info");
    }

    void GridInfoHelper::GetGridInfo()
    {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL( finished(QNetworkReply*) ), SLOT( ReplyRecieved(QNetworkReply*) ));
        manager->get(QNetworkRequest(url_));
    }

    void GridInfoHelper::ReplyRecieved(QNetworkReply *reply)
    {
        QMap <QString, QVariant> grid_info_map;
        QXmlStreamReader xml_reader;
        xml_reader.addData(reply->readAll());

        while (!xml_reader.atEnd())
        {
            QXmlStreamReader::TokenType token = xml_reader.readNext();

            if (xml_reader.hasError())
                break;

            if (xml_reader.name() != "gridinfo" && !xml_reader.name().isEmpty())
                grid_info_map[xml_reader.name().toString()] = xml_reader.readElementText();
        }

        if (grid_info_map.contains("login"))
            emit GridInfoDataRecieved(grid_info_map);
        else
            emit GridInfoDataRecieved(QMap <QString, QVariant>());
    }

}