// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_INetMessageListener_h
#define incl_INetMessageListener_h

//#include "OpenSimProtocolModule_API.h"

/// Interface that is implemented by an object that is interested in receiving network messages.
/// Register network listener objects to a NetMessageManager.
class MODULE_API INetMessageListener
{
public:
	INetMessageListener() {}
	virtual ~INetMessageListener() {}

	/// Called for each network message received. The callee can process the inputted message
    /// any way he wants.
	/// @param msgID The type of the message.
	/// @param msg The actual message contents.
	virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg) = 0;

	/// Called for each network message that is sent. This callback is provided just for 
    /// debugging/stats collecting. \todo Provide a converter object that converts a NetOutMessage to a NetMessage base object or a NetInMessage object to allow reading the contents.
	/// @param msg The message contents.
    virtual void OnNetworkMessageSent(const NetOutMessage *msg) {}
};

#endif
