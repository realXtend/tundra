#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

/// Network message informing that client has joined the server.
struct MsgClientJoined
{
	MsgClientJoined()
	{
		InitToDefault();
	}

	MsgClientJoined(const char *data, size_t numBytes)
	{
		InitToDefault();
		kNet::DataDeserializer dd(data, numBytes);
		DeserializeFrom(dd);
	}

	void InitToDefault()
	{
		reliable = defaultReliable;
		inOrder = defaultInOrder;
		priority = defaultPriority;
	}

	enum { messageID = 102 };
	static inline const char * Name() { return "ClientJoined"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 userID;
	QString username;

	inline size_t Size() const
	{
		return (size_t)kNet::VLE8_16_32::GetEncodedBitLength(userID) + 2 + username.toUtf8().size()*1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.AddVLE<kNet::VLE8_16_32>(userID);
		WriteUtf8String(dst, username);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		userID = src.ReadVLE<kNet::VLE8_16_32>();
		if (src.BytesLeft() > 0)
		    username = ReadUtf8String(src);
	}
};
