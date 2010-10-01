#pragma once

#include "kNet.h"

struct MsgEntityIDCollision
{
	MsgEntityIDCollision()
	{
		InitToDefault();
	}

	MsgEntityIDCollision(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 115; }
	static inline const char *Name() { return "EntityIDCollision"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 oldEntityID;
	u32 newEntityID;

	inline size_t Size() const
	{
		return 4 + 4;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u32>(oldEntityID);
		dst.Add<u32>(newEntityID);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		oldEntityID = src.Read<u32>();
		newEntityID = src.Read<u32>();
	}

};

