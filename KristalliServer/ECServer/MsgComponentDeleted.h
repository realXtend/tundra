#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgComponentDeleted
{
	MsgComponentDeleted()
	{
		InitToDefault();
	}

	MsgComponentDeleted(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 123; }
	static inline const char *Name() { return "ComponentDeleted"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;
	u8 entityUUID[16];
	std::vector<s8> componentTypeName;
	std::vector<s8> componentName;

	inline size_t Size() const
	{
		return 4 + 16*1 + 1 + componentTypeName.size()*1 + 1 + componentName.size()*1;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.AddArray<u8>(entityUUID, 16);
		dst.Add<u8>(componentTypeName.size());
		if (componentTypeName.size() > 0)
			dst.AddArray<s8>(&componentTypeName[0], componentTypeName.size());
		dst.Add<u8>(componentName.size());
		if (componentName.size() > 0)
			dst.AddArray<s8>(&componentName[0], componentName.size());
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		src.ReadArray<u8>(entityUUID, 16);
		componentTypeName.resize(src.Read<u8>());
		if (componentTypeName.size() > 0)
			src.ReadArray<s8>(&componentTypeName[0], componentTypeName.size());
		componentName.resize(src.Read<u8>());
		if (componentName.size() > 0)
			src.ReadArray<s8>(&componentName[0], componentName.size());
	}

};

