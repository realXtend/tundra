// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"

#include "ApplicationManager.h"
#include <QDesktopServices>

namespace MumbleVoip
{
    int ApplicationManager::start_count_ = 0;

    ApplicationManager::ApplicationManager()
    {
    }

    // static
    void ApplicationManager::StartMumbleClient(const QString& server_url)
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
        ApplicationManager::start_count_++;
    }

    //static
    int ApplicationManager::StartCount()
    {
        return ApplicationManager::start_count_;
    }

} // namespace MumbleVoip
