#include "StableHeaders.h"
#include "ConnectionManager.h"

#include <QDesktopServices>

namespace MumbleVoip
{

    ConnectionManager::ConnectionManager()
    {

    }

    void ConnectionManager::OpenConnection(ServerInfo info)
    {
        // todo: IMPLEMENT
    }

    void ConnectionManager::CloseConnection(ServerInfo info)
    {
        // todo: IMPLEMENT
    }

    // static
    void ConnectionManager::StartMumbleClient(const QString& server_url)
    {
        QUrl url(server_url);
        if (!url.isValid())
        {
            QString error_message = QString("Url '%1' is invalid.").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
        
        if (! QDesktopServices::openUrl(server_url))
        {
            QString error_message = QString("Cannot find handler application for url: %1").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
    }

} // end of namespace: MumbleVoip

