/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef INetworkServerListener_h
#define INetworkServerListener_h

/// An interface implementable by the owner of a network server to receive notifications about connection-related events.
/// @note Do not call NetworkServer::ProcessMessages from any of these methods. This might cause infinite recursion.
class INetworkServerListener
{
public:
	/// Called to query whether the new connection should be accepted or not, but only if the server operates in UDP mode.
	/// @return If the implementor of this listener decides the connection should be accepted, it should return true.
	///       If it returns false, the connection attempt will be ignored.
	virtual bool NewConnectionAttempt(const EndPoint &endPoint, Datagram &datagram)
	{
		/// @note The default implementation of this method is to accept the connection. Be sure to override this if
		///       custom access control is needed.
		return true;
	}

	/// Called to notify the listener that a new connection has been established.
	/// The application is expected to at least register a message listener for the new connection.
	virtual void NewConnectionEstablished(MessageConnection *connection) = 0;

	/// Called to notify the listener that the given client has disconnected.
	virtual void ClientDisconnected(MessageConnection *connection)
	{
		/// The default action is to not do anything.
	}
};

#endif
