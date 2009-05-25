#ifndef incl_NewsFeedServerConnection_h
#define incl_NewsFeedServerConnection_h

#include "Foundation.h"

/**
 *  Just an idea..
 *
 *  What if we could have news message acstraction for receiving 
 *  news messages from different sources so user can get message from
 *  interested events.
 */
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

	/**
	 *  News feed server connection for eg. Atom, RSS and Web syndication services
	 */
	class NewsFeedServerConnection
	{
	public:
		NewsFeedServerConnection(void);
		~NewsFeedServerConnection(void);

        //commented out 'cause does not build at the moment and is not needed currently
		//void Open(CommunicationSettings c); 
		void Close();
		void Subscribe(/*resource*/);
		void Unsubscribe(/*resource*/);
	};

} // end of namespace Communication

#endif // incl_NewsFeedServerConnection_h