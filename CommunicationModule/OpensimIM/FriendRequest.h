#ifndef incl_Communication_OpensimIM_FriendRequest_h
#define incl_Communication_OpensimIM_FriendRequest_h

#include "Foundation.h"
#include "..\interface.h"

namespace OpensimIM
{
	class FriendRequest : public Communication::FriendRequestInterface
	{
	public:
		FriendRequest(Foundation::Framework* framework, const QString &id, const QString &name, const QString &transaction_id, const QString &calling_card_folder_id);
		virtual QString GetOriginatorName() const;
		virtual QString GetOriginatorID() const;
		virtual State GetState() const;
		virtual void Accept();
		virtual void Reject();
	private:
		Foundation::Framework* framework_;
		State state_;
		QString id_;
		QString name_;
		QString transaction_id_; //!  @todo Find out what to put here... probably we get this information from friend request packet.
		QString	calling_card_folder_id_;
	};
	typedef std::vector<FriendRequest*> FriendRequestVector;
	
} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_FriendRequest_h
