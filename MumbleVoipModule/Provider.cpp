#include "StableHeaders.h"
#include "Provider.h"
#include "Session.h"
#include "MumbleVoipModule.h"
#include "ServerInfoProvider.h"
#include "EventManager.h"
#include "NetworkEvents.h" // For network events

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Provider::Provider(Foundation::Framework* framework) :
            framework_(framework),
            description_("Mumble in-world voice"),
            session_(0),
            server_info_(0),
            server_info_provider_(0)
        {
            server_info_provider_ = new ServerInfoProvider(framework);
            connect(server_info_provider_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );
        }

        Provider::~Provider()
        {
            SAFE_DELETE(session_);
            SAFE_DELETE(server_info_provider_);
            SAFE_DELETE(server_info_);
        }

        void Provider::Update(f64 frametime)
        {
            if (session_)
                session_->Update(frametime);
        }
        
        bool Provider::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
        {
            if (server_info_provider_)
                server_info_provider_->HandleEvent(category_id, event_id, data);

            if (category_id == networkstate_event_category_)
            {
                switch (event_id)
                {
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                    CloseSession();
                    break;
                }
            }

            return false;
        }

        Communications::InWorldVoice::SessionInterface* Provider::Session()
        {
            if (session_ && session_->GetState() == Session::STATE_CLOSED)
                SAFE_DELETE(session_) //! \todo USE SHARED PTR, SOMEONE MIGHT HAVE POINTER TO SESSION OBJECT !!!!

            if (!session_)
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

            networkstate_event_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

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

        void Provider::CloseSession()
        {
            if (session_)
                session_->Close();
            emit SessionUnavailable();
        }

        QList<QString> Provider::Statistics()
        {
            if (!session_)
            {
                QList<QString> lines;
                return lines;
            }
            else
                return session_->Statistics();
        }


    } // InWorldVoice

} // MumbleVoip
