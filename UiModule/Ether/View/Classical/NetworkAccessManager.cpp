#include "StableHeaders.h"
#include <QtNetwork>
#include "NetworkAccessManager.h"
#include "CablebeachReply.h"

namespace CoreUi
{
    namespace Classical
    {
        NetworkAccessManager::NetworkAccessManager(QNetworkAccessManager *manager, QObject *parent): QNetworkAccessManager(parent)
        {
            setCache(manager->cache());
            setCookieJar(manager->cookieJar());
            setProxy(manager->proxy());
            setProxyFactory(manager->proxyFactory());
        }

        QNetworkReply *NetworkAccessManager::createRequest(
            QNetworkAccessManager::Operation operation, const QNetworkRequest &request,
            QIODevice *device)
        {
            if (request.url().scheme() != "cablebeach")
                return QNetworkAccessManager::createRequest(operation, request, device);

            if (operation == GetOperation)
            {
                qDebug() << "Emitting url" << request.url().toString() << endl;
                emit WebLoginUrlRecived(request.url());
                return new CablebeachReply(request.url());
            }
            else
                return QNetworkAccessManager::createRequest(operation, request, device);
        }
    }
}