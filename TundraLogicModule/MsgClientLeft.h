#pragma once

#include "kNet.h"

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
		reliable = true;
		inOrder = true;
		priority = 100;
	}

    enum { messageID = 103 };
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

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u8>(userID);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		userID = src.Read<u8>();
	}

};

