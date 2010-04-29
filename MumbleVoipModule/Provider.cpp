#include "StableHeaders.h"
#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "ServerObserver.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Provider::Provider(Foundation::Framework* framework) :
            description_("Mumble in-world voice"),
            session_(0),
            server_info_(0)
        {
            server_observer_ = new ServerObserver(framework);
            connect(server_observer_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );
        }

        Provider::~Provider()
        {
            SAFE_DELETE(session_);
            SAFE_DELETE(server_observer_);
        }

        Communications::InWorldVoice::SessionInterface* Provider::Session()
        {
            if (session_ == 0)
            {
                if (!server_info_)
                    return 0;
                session_ = new MumbleVoip::InWorldVoice::Session(framework_, *server_info_);
            }
            return session_;
        }

        QString& Provider::Description()
        {
            return description_;
        }

        void Provider::OnMumbleServerInfoReceived(ServerInfo info)
        {
            server_info_ = new ServerInfo(info);

            emit SessionAvailable();
        }

    } // InWorldVoice

} // MumbleVoip
