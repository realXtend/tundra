// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <RexLogicModule.h>
#include "FriendRequest.h"
#include "ModuleManager.h"
#include "WorldStream.h"
#include "Framework.h"
#include "CoreException.h"

#include "MemoryLeakCheck.h"

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
            throw Exception("Cannot accept Opensim friend request, RexLogicModule is not found");
        RexLogic::WorldStreamConnectionPtr connection = rexlogic_->GetServerConnection();

        if ( connection == NULL )
            throw Exception("Cannot accept Opensim friend request rex server connection is not found");

        if ( !connection->IsConnected() )
            throw Exception("Cannot accept Opensim friend request, rex server connection is not established");

        connection->SendAcceptFriendshipPacket(RexUUID( friend_id_.toStdString() ), RexUUID( calling_card_folder_id_.toStdString() ));

        //! @todo Update contact list

        state_ = STATE_ACCEPTED;
        emit ( FriendRequestAccepted(this) );
    }

    void FriendRequest::Reject()
    {
        RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

        if (rexlogic_ == NULL)
            throw Exception("Cannot reject Opensim friend request, RexLogicModule is not found");
        RexLogic::WorldStreamConnectionPtr connection = rexlogic_->GetServerConnection();

        if ( connection == NULL )
            throw Exception("Cannot reject Opensim friend request rex server connection is not found");

        if ( !connection->IsConnected() )
            throw Exception("Cannot reject Opensim friend request, rex server connection is not established");

        connection->SendDeclineFriendshipPacket(RexUUID( friend_id_.toStdString() ) );
        state_ = STATE_REJECTED;
        emit FriendRequestRejected(this);
    }

} // end of namespace: OpensimIM
