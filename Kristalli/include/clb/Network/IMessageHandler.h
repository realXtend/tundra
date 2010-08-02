/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef IMessageHandler_h
#define IMessageHandler_h

#include "clb/Core/Types.h"

class MessageConnection;

typedef unsigned long message_id_t;

/// IMessageHandler is a callback object used by the MessageConnection to invoke the main application
/// whenever a message has been received.
class IMessageHandler
{
public:
	virtual ~IMessageHandler() {}

	/// Called whenever the network stack has received a message that the application
	/// needs to process.
	virtual void HandleMessage(MessageConnection *source, message_id_t id, const char *data, size_t numBytes) = 0;

	/// Called by the network library to ask the application to produce a content ID
	/// associated with the given message. If the application returns 0, the message doesn't
	/// have a ContentID and it is processed normally.
	/// The ContentID of the message is used to determine if a message replaces another.
	virtual u32 ComputeContentID(message_id_t id, const char *data, size_t numBytes)
	{
		// The default behavior is to not have a content ID on any message.
		return 0;
	}
};

#endif
