// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ServerInfoProvider.h"

#include "WorldStream.h"
#include "MumbleVoipModule.h"
#include "EventManager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{

    ServerInfoProvider::ServerInfoProvider(Foundation::Framework* framework) :
        framework_event_category_(0),
        networkstate_event_category_(0),
        framework_(framework),
        server_info_request_manager_(new QNetworkAccessManager())
    {
        connect(server_info_request_manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnMumbleServerInfoHttpResponse(QNetworkReply*)));
    }

    ServerInfoProvider::~ServerInfoProvider()
    {
        SAFE_DELETE(server_info_request_manager_);
    }

    bool ServerInfoProvider::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        if (!framework_event_category_ && framework_)
           framework_event_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");

        if (!networkstate_event_category_ && framework_)
           networkstate_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (category_id == framework_event_category_)
        {
            switch (event_id)
            {
            case Foundation::WORLD_STREAM_READY:
                {
                    ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                    if (event_data)
                    {
                        current_world_stream_ = event_data->WorldStream;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (category_id == networkstate_event_category_)
        {
            switch (event_id)
            {
            case ProtocolUtilities::Events::EVENT_CAPS_FETCHED:
                {
                    if (current_world_stream_)
                    {
                        QString cap_mumble_server_info_url = current_world_stream_->GetCapability("mumble_server_info");
                        if (cap_mumble_server_info_url.size() > 0)
                        {
                            ProtocolUtilities::ClientParameters client_info = current_world_stream_->GetInfo();
                            QString agent_id = QString(client_info.agentID.ToString().c_str());
                            RequestMumbleServerInfo(cap_mumble_server_info_url, agent_id);
                        }
                    }
                }
                break;
            }
        }

        //if (category_id == networkstate_event_category_)
        //{
        //    switch (event_id)
        //    {
        //    case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
        //        break;
        //    case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
        //        break;
        //    }
        //}

        return false;
    }

    void ServerInfoProvider::RequestMumbleServerInfo(const QString &url, const QString &agent_id)
    {
        //QString path = "mumble_server_info";
        //QUrl url(grid_url);
        //if (url.scheme().length() == 0)
        //    url.setUrl(QString("http://%1").arg(grid_url));
        //url.setPath(path);

        QNetworkRequest request(url);
        request.setRawHeader("avatar_uuid",agent_id.toAscii());
        server_info_request_manager_->get(request);
    }

    void ServerInfoProvider::OnMumbleServerInfoHttpResponse(QNetworkReply* reply)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            QString message = QString("Mumble server info not available for server %1").arg(reply->url().toString());
            MumbleVoipModule::LogInfo(message.toStdString());
            reply->abort();
            reply->deleteLater();
            return;
        }

        ServerInfo info;
        info.server = reply->rawHeader("Mumble-Server");
        info.version = reply->rawHeader("Mumble-Version");
        info.channel = reply->rawHeader("Mumble-Channel");
        info.user_name = reply->rawHeader("Mumble-User");
        info.password = reply->rawHeader("Mumble-Password");
        info.avatar_id = reply->rawHeader("Mumble-Avatar-Id");
        info.context_id = reply->rawHeader("Mumble-Context-Id");

        QString message = QString("Mumble server info received for %1").arg(reply->url().toString());
        MumbleVoipModule::LogDebug(message.toStdString());

        emit MumbleServerInfoReceived(info);
    }

} // end of namespace: MumbleVoip
