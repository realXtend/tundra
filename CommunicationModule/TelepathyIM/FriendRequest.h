#ifndef incl_Communication_TelepathyIM_FriendRequest_h
#define incl_Communication_TelepathyIM_FriendRequest_h

#include "..\interface.h"

namespace TelepathyIM
{
	/**
	 *  This class is only used by CommunicationService object. Do not use this
	 *  directly!
     *
	 *  A received friend request. This can be accepted or rejected. If Accpet() methos is called1
	 *  then Connection object emits NewContact signal.
	 *
	 *
	 */
	class FriendRequest : public Communication::FriendRequestInterface 
	{
		Q_OBJECT
	public:
		FriendRequest();
		virtual QString GetOriginatorName() const;
		virtual QString GetOriginatorID() const;
		virtual Communication::FriendRequestInterface::State GetState() const;
		virtual void Accept();
		virtual void Reject();
	protected:
		State state_;
	};

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_FriendRequest_h