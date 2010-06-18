// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Provider.h"
#include "Session.h"
#include "RexLogicModule.h"
#include "ModuleManager.h"
#include "WorldStream.h"

namespace RexLogic
{
    namespace InWorldChat
    {
        Provider::Provider(Foundation::Framework* framework) : 
            framework_(framework),
            description_("RexLogic module opensim chat provider"),
            session_(0)
        {
            RegisterToCommunicationsService();
        }

        Provider::~Provider()
        {
            SAFE_DELETE(session_);
            foreach(InWorldChat::Session* s, closed_sessions_)
                SAFE_DELETE(s);

            closed_sessions_.clear();
        }

        bool Provider::HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data)
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                session_ = new InWorldChat::Session();
                connect(session_, SIGNAL(UserEnteredText(const QString&)), SLOT(SendChatMessgeToServer(const QString&)) );
                emit SessionAvailable();
            }

            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            {
                if (session_)
                {
                    disconnect(session_);
                    session_->Close();
                    closed_sessions_.append(session_);
                    session_ = 0;
                }
                emit SessionUnavailable();
            }
            return false;
        }

        Communications::InWorldChat::SessionInterface* Provider::Session() const
        {
            return session_;
        }

        QString Provider::Description()const
        {
            return description_;
        }

        void Provider::RegisterToCommunicationsService()
        {
            boost::shared_ptr<Communications::ServiceInterface> comm =
                framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
            if (comm)
                comm->Register(*this);
        }

        void Provider::HandleIncomingChatMessage(const QString& from_uuid, const QString& from_name, const QString& text)
        {
            if (session_)
                session_->HandleIncomingTextMessage(from_uuid, from_name, text);
        }

        void Provider::SendChatMessgeToServer(const QString& text)
        {
            RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(
                framework_->GetModuleManager()->GetModule("RexLogic").lock().get());
            if (!rexlogic_)
                throw Exception("Cannot send text message, RexLogicModule is not found");

            WorldStreamPtr connection = rexlogic_->GetServerConnection();
            if (!connection)
                throw Exception("Cannot send text message, rex server connection is not found");

            if (!connection->IsConnected() )
                throw Exception("Cannot send text message, rex server connection is not established");

            connection->SendChatFromViewerPacket( std::string(text.toUtf8()) );
        }
    } // InWorldChat
} // RexLogic
