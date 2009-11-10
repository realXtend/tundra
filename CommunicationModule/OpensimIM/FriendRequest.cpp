#include <RexLogicModule.h>
#include "FriendRequest.h"

namespace OpensimIM
{
	FriendRequest::FriendRequest(Foundation::Framework* framework, const QString &id, const QString &name, const QString &calling_card_folder_id): framework_(framework), friend_id_(id), name_(name), calling_card_folder_id_(calling_card_folder_id), state_(STATE_PENDING)
	{
	}

	QString FriendRequest::GetOriginatorName() const
	{
		return name_;
	}

	QString FriendRequest::GetOriginatorID() const
	{
		return friend_id_;
	}

	Communication::FriendRequestInterface::State FriendRequest::GetState() const
	{
		return state_;
	}

	void FriendRequest::Accept()
	{
		RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot accept Opensim friend request, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot accept Opensim friend request rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot accept Opensim friend request, rex server connection is not established");

		connection->SendAcceptFriendshipPacket(RexTypes::RexUUID( friend_id_.toStdString() ), RexTypes::RexUUID( calling_card_folder_id_.toStdString() ));

		//! @todo Update contact list

		state_ = STATE_ACCEPTED;
		emit ( FriendRequestAccepted(this) );
	}

	void FriendRequest::Reject()
	{
		RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot reject Opensim friend request, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot reject Opensim friend request rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot reject Opensim friend request, rex server connection is not established");

		connection->SendDeclineFriendshipPacket(RexTypes::RexUUID( friend_id_.toStdString() ) );
		state_ = STATE_REJECTED;
		emit FriendRequestRejected(this);
	}

} // end of namespace: OpensimIM
