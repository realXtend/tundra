#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgEntityDeleted
{
	MsgEntityDeleted()
	{
		InitToDefault();
	}

	MsgEntityDeleted(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 122; }
	static inline const char *Name() { return "EntityDeleted"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;
	u8 entityUUID[16];

	inline size_t Size() const
	{
		return 4 + 16*1;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.AddArray<u8>(entityUUID, 16);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		src.ReadArray<u8>(entityUUID, 16);
	}

};

