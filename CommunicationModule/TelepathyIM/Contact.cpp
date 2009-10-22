#include "Contact.h"

namespace TelepathyIM
{

	Contact::Contact(Tp::ContactPtr tp_contact): tp_contact_(tp_contact)
	{
		//! @todo IMPLEMENT
	}

	Contact::~Contact()
	{
		//! @todo IMPLEMENT
	}

	QString Contact::GetID() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	QString Contact::GetName() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	void Contact::SetName(const QString& name)
	{
		//! @todo IMPLEMENT
	}

	QString Contact::GetPresenceStatus() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	QString Contact::GetPresenceMessage() const
	{
		//! @todo IMPLEMENT
		return "";
	}

} // end of namespace: TelepathyIM
	