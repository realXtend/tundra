#ifndef incl_NewsFeedServerConnection_h
#define incl_NewsFeedServerConnection_h

#include "CoreStdIncludes.h"
#include "CommunicationService.h"

// OPEN QUESTIONS:  
// * Where subscribe lists are stored?

namespace Communication
{
	class NewsMessage
	{
		// source
		// timestamp
		// title
		// content:
		// -text
		// -attachments
	};

	// News feed server connection for eg. Atom, RSS and Web syndication services
	// EVENTS: 
	// - NewsMessage
	class NewsFeedServerConnection
	{
	public:
		NewsFeedServerConnection(void);
		~NewsFeedServerConnection(void);

		void Open(Credentials c); 
		void Close();
		void Subscribe(/*resource*/);
		void Unsubscribe(/*resource*/);
	};

} // end of namespace Communication

#endif // incl_NewsFeedServerConnection_h