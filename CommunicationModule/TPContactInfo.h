#ifndef incl_TPContactInfo_h
#define incl_TPContactInfo_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	// A single contact information of individual contact.
	// Eg. jabber id:  "jid": "myjabberid@myjabberprovider.com"
	class TPContactInfo
	{
	public:
		virtual void SetType(std::string type);
		virtual std::string GetType();
		virtual void SetValue(std::string value);
		virtual std::string GetValue();
	private:
		std::string type_;
		std::string value_;
	};

}

#endif // incl_TPContactInfo_h