#include "StableHeaders.h"
#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "ConnectionManager.h"
#include "ServerObserver.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Provider::Provider(Foundation::Framework* framework) :
            description_("Mumble in-world voice"),
            session_(0),
            connection_manager_(0),
            server_info_(0)
        {
            connection_manager_ = new ConnectionManager(framework);
            server_observer_ = new ServerObserver(framework);
            connect(server_observer_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );
        }

        Provider::~Provider()
        {
            SAFE_DELETE(server_observer_);
            SAFE_DELETE(connection_manager_);
        }

        Communications::InWorldVoice::SessionInterface* Provider::Session()
        {
            if (session_ == 0)
                return 0;
            else
                return session_;
        }

        QString& Provider::Description()
        {
            return description_;
        }

        void Provider::OnMumbleServerInfoReceived(ServerInfo info)
        {
            server_info_ = new ServerInfo(info);

//            emit SessionAvailable();
        }

    } // InWorldVoice

} // MumbleVoip
