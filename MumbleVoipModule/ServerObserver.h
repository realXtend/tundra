// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ServerObserver_h
#define incl_MumbleVoipModule_ServerObserver_h


#include <QObject>
#include "CoreTypes.h"
#include "ServerInfo.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace Foundation
{
    class Framework;
}

namespace Foundation
{
    class EventDataInterface;
}

namespace MumbleVoip
{
    /**
     *  Requests mumble server info when user connects to world.
     *  Emits MumbleServerInfoReceived signal when server info is available.
     *
     * \todo Rename to 'ServerInfoProvider'
     */
    class ServerObserver : public QObject
    {
        Q_OBJECT

    public:
        ServerObserver(Foundation::Framework* framework);
        virtual ~ServerObserver();

        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    signals:
        //! Emited when server info was successfully found
        void MumbleServerInfoReceived(ServerInfo info);

    private:
        void RequestMumbleServerInfo(const QString &grid_url, const QString &agent_id);

        Foundation::Framework* framework_;
        QNetworkAccessManager* server_info_request_manager_;
        event_category_id_t framework_event_category_; 
        event_category_id_t networkstate_event_category_;

    private slots:
        void OnMumbleServerInfoHttpResponse(QNetworkReply* reply);
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ServerObserver_h
