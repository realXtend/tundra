// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_GridInfoHelper_h
#define incl_Protocol_GridInfoHelper_h

#include <QObject>
#include <QMap>
#include <QString>
#include <QUrl>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace ProtocolUtilities
{
    class GridInfoHelper : public QObject
    {

    Q_OBJECT

    public:
        GridInfoHelper(QObject *parent, QUrl url);

    public slots:
        void GetGridInfo();

    private slots:
        void ReplyRecieved(QNetworkReply *reply);

    private:
        QUrl url_;

    signals:
        void GridInfoDataRecieved(QMap<QString, QVariant>);

    };

}

#endif // incl_Protocol_GridInfoHelper_h