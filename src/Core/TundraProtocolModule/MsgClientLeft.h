#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

struct MsgClientLeft
{
	MsgClientLeft()
	{
		InitToDefault();
	}

	MsgClientLeft(const char *data, size_t numBytes)
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

	enum { messageID = 103 };
	static inline const char * const Name() { return "ClientLeft"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	u8 userID;

	inline size_t Size() const
	{
		return 1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u8>(userID);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		userID = src.Read<u8>();
	}

};

