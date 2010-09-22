#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgRemoveEntity
{
	MsgRemoveEntity()
	{
		InitToDefault();
	}

	MsgRemoveEntity(const char *data, size_t numBytes)
	{
		InitToDefault();
		DataDeserializer dd(data, numBytes);
		DeserializeFrom(dd);
	}

	void InitToDefault()
	{
		reliable = true;
		inOrder = true;
		priority = 100;
	}

	static inline u32 MessageID() { return 111; }
	static inline const char *Name() { return "RemoveEntity"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;

	inline size_t Size() const
	{
		return 4;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		entityID = src.Read<u32>();
	}

};

