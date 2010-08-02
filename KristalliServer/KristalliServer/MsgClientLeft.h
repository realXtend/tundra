#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgClientLeft
{
	MsgClientLeft()
	{
		InitToDefault();
	}

	MsgClientLeft(const char *data, size_t numBytes)
	{
		InitToDefault();
		DataDeserializer dd(data, numBytes);
		DeserializeFrom(dd);
	}

	void InitToDefault()
	{
		reliable = true;
		inOrder = false;
		priority = 100;
	}

	static inline u32 MessageID() { return 103; }
	static inline const char *Name() { return "ClientLeft"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u8 userID;

	inline size_t Size() const
	{
		return 1;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u8>(userID);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		userID = src.Read<u8>();
	}

};

