#include "StableHeaders.h"
#include "Foundation.h"

#include "Contact.h"


namespace TpQt4Communication
{
	Contact::Contact(Tp::ContactPtr tp_contact)
	{
		LogInfo("Create Contact object");
		tp_contact_ = tp_contact;
		ConnectSignals();
	}

	void Contact::ConnectSignals()
	{
		//QObject::connect(tp_contact_.data(),
  //          SIGNAL(simplePresenceChanged(const QString &, uint, const QString &)),
  //          SLOT(OnContactChanged()));
		//QObject::connect(tp_contact_,
  //          SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
  //          SLOT(OnContactChanged()));
		//QObject::connect(tp_contact_,
  //          SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
  //          SLOT(OnContactChanged()));
		//QObject::connect(tp_contact_,
  //          SIGNAL(blockStatusChanged(bool)),
  //          SLOT(OnContactChanged()));
	}

	void Contact::OnContactChanged()
	{
		LogInfo("Contact state changed");

		QString status =  tp_contact_->presenceStatus();
		
		switch( tp_contact_->subscriptionState() )
		{
			case Tp::Contact::PresenceStateAsk: break;
			case Tp::Contact::PresenceStateNo: break;
			case Tp::Contact::PresenceStateYes: break;
		}
		
		if (tp_contact_->isBlocked())
		{
			// User has blocked this contact
		}
	}

} // end of namespace: Communication