#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QUrl>

namespace CoreUi
{
    namespace Classical
    {
        class NetworkAccessManager : public QNetworkAccessManager
        {

        Q_OBJECT

        public:
            NetworkAccessManager(QNetworkAccessManager *oldManager, QObject *parent = 0);

        protected:    
            QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *device);

        signals:
            void WebLoginUrlRecived(QUrl);

        };
    }
}

#endif