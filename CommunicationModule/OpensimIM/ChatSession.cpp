#include "ChatSession.h"
#include "RexLogicModule.h" 

namespace OpensimIM
{
	ChatSession::ChatSession(Foundation::Framework* framework): framework_(framework)
	{
		//! \todo catch RexNetMsgChatFromSimulator
	}

	void ChatSession::SendMessage(const QString &text)
	{
		RexLogic::RexLogicModule *rexlogic_;
        rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot send text message, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot send text message, rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot send text message, rex server connection is not established");

		connection->SendChatFromViewerPacket( text.toStdString() );
	}

	void ChatSession::Close()
	{
		//! \todo IMPLEMENT
	}

} // end of namespace: OpensimIM
