#ifndef incl_Communication_OpensimIM_Contact_h
#define incl_Communication_OpensimIM_Contact_h

#include "Foundation.h"
#include "NetworkEvents.h"
#include "..\interface.h"
//#include "ChatSessionParticipant.h"

namespace OpensimIM
{
	class Contact : public Communication::ContactInterface
	{
	public:
		Contact(const QString &id, const QString &name);

		//! UUID of this contact
		virtual QString GetID() const;

		//! Provides name of this contact item
		virtual QString GetName() const;

		//! Not supported by Opensim
		virtual void SetName(const QString& name); 

		//! Values "online", "offline"
		virtual QString GetPresenceStatus() const;

		//! Not supported by Opensim
		virtual QString GetPresenceMessage() const;
	private:

		//! UUID of this contact
		QString id_;

		QString name_;

		bool online_;
		bool is_typing_; // probably will be used on ChatSessionParticipant class

		//bool rights_to_see_users_online_status;
		//bool rights_to_locate_user_;
		//bool right_to_use_users_objects_;
	};
	typedef std::vector<Contact*> ContactVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_Contact_h
