#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgCreateEntity
{
	MsgCreateEntity()
	{
		InitToDefault();
	}

	MsgCreateEntity(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 110; }
	static inline const char *Name() { return "CreateEntity"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;
	std::vector<u8> componentData;

	inline size_t Size() const
	{
		return 4 + 4 + componentData.size()*1;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.Add<u32>(componentData.size());
		if (componentData.size() > 0)
			dst.AddArray<u8>(&componentData[0], componentData.size());
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		componentData.resize(src.Read<u32>());
		if (componentData.size() > 0)
			src.ReadArray<u8>(&componentData[0], componentData.size());
	}

};

