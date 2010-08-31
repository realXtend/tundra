/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef FragmentedTransferManager_h
#define FragmentedTransferManager_h

#include <map>
#include <vector>
#include <list>

class NetworkMessage;

/// @internal Manages the allocation of transferIDs to fragmented message transfers and tracks which of the fragments have
/// successfully been sent over to the receiver.
class FragmentedSendManager
{
public:
	struct FragmentedTransfer
	{
		int id;

		/// The total number of fragments in this message.
		size_t totalNumFragments;

		std::list<NetworkMessage*> fragments;

		void AddMessage(NetworkMessage *message);
	};

	typedef std::list<FragmentedTransfer> TransferList;
	TransferList transfers;

	/// Returns a new FragmentedTransfer. A transferID for this transfer will not have been allocated here.
	/// When sending the message is finished, call FreeFragmentedTransfer.
	FragmentedTransfer *AllocateNewFragmentedTransfer();
	void RemoveMessage(FragmentedTransfer *transfer, NetworkMessage *message);

	/// @return True if the allocation succeeded, false otherwise.
	bool AllocateFragmentedTransferID(FragmentedTransfer &transfer);

private:
	void FreeFragmentedTransfer(FragmentedTransfer *transfer);
};

/// @internal Receives message fragments and assembles fragments to complete messages when they are finished.
class FragmentedReceiveManager
{
public:
	struct ReceiveFragment
	{
		int fragmentIndex;

		std::vector<char> data;
	};

	struct ReceiveTransfer
	{
		int transferID;

		int numTotalFragments;

		std::vector<ReceiveFragment> fragments;
	};

	std::vector<ReceiveTransfer> transfers;

	void NewFragmentStartReceived(int transferID, int numTotalFragments, const char *data, size_t numBytes);
	bool NewFragmentReceived(int transferID, int fragmentNumber, const char *data, size_t numBytes);
	void AssembleMessage(int transferID, std::vector<char> &assembledData);
	void FreeMessage(int transferID);
};

#endif
