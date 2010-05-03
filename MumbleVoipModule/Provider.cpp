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
            framework_(framework),
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
            SAFE_DELETE(server_info_);
        }

        void Provider::Update(f64 frametime)
        {
            if (session_)
                session_->Update(frametime);
        }
        
        bool Provider::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
        {
            if (server_observer_)
                server_observer_->HandleEvent(category_id, event_id, data);
            return false;
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

            if (framework_ &&  framework_->GetServiceManager())
            {
                boost::shared_ptr<Communications::ServiceInterface> comm = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
                if (comm.get())
                {
                    comm->Register(*this);
                }
                return;
            }
        }
    } // InWorldVoice

} // MumbleVoip
