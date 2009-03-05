// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_INetMessageListener_h
#define incl_INetMessageListener_h

/// Interface that is implemented by an object that is interested in receiving network messages. Register network listener
/// objects to a NetMessageManager.
class INetMessageListener
{
public:
	INetMessageListener() {}
	virtual ~INetMessageListener() {}

	/// Called for each network message received.
	/// @param msgID The type of the message.
	/// @param msg The actual message contents.
	virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg) = 0;
};

#endif
