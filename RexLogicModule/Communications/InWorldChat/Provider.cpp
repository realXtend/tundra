#include "StableHeaders.h"
#include "Provider.h"
#include "Session.h"

namespace RexLogic
{
    namespace InWorldChat
    {
        Provider::Provider(Foundation::Framework* framwwork) : 
            framework_(framwwork),
            description_("RexLogic module opensim chat provider"),
            session_(0)
        {
            session_ = new InWorldChat::Session();
            Register();
        }

        Provider::~Provider()
        {

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

    } // InWorldChat

} // RexLogic
