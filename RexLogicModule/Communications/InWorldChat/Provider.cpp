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
            session_ = new InWorldChat::Session();
            connect(session_, SIGNAL(UserEnteredText(const QString&)), SLOT(SendChatMessgeToServer(const QString&)) );
            Register();
        }

        Provider::~Provider()
        {
            SAFE_DELETE(session_);
        }

        Communications::InWorldChat::SessionInterface* Provider::Session()
        {
            //if (session_ && session_->GetState() == Session::STATE_CLOSED)
            //    SAFE_DELETE(session_) //! \todo USE SHARED PTR, SOMEONE MIGHT HAVE POINTER TO SESSION OBJECT !!!!

            if (!session_)
            {
                session_ = new InWorldChat::Session();
            }
            return session_;
        }

        QString& Provider::Description()
        {
            return description_;
        }

        void Provider::Register()
        {
            boost::shared_ptr<Communications::ServiceInterface> comm = framework_->GetServiceManager()->GetService<Communications::ServiceInterface>(Foundation::Service::ST_Communications).lock();
            if (comm.get())
            {
                comm->Register(*this);
            }
            return;
        }

        void Provider::HandleIncomingChatMessage(const QString& from_uuid, const QString& from_name, const QString& text)
        {
            if (!session_)
                return;

            session_->HandleIncomingTextMessage(from_uuid, from_name, text);
        }

        void Provider::SendChatMessgeToServer(const QString& text)
        {
            RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

            if (rexlogic_ == NULL)
                throw Exception("Cannot send text message, RexLogicModule is not found");
            WorldStreamPtr connection = rexlogic_->GetServerConnection();

            if ( connection == NULL )
                throw Exception("Cannot send text message, rex server connection is not found");

            if ( !connection->IsConnected() )
                throw Exception("Cannot send text message, rex server connection is not established");

            connection->SendChatFromViewerPacket( std::string(text.toUtf8()) );
    }

    } // InWorldChat

} // RexLogic
