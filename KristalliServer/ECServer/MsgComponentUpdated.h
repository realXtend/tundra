#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgComponentUpdated
{
	MsgComponentUpdated()
	{
		InitToDefault();
	}

	MsgComponentUpdated(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 121; }
	static inline const char *Name() { return "ComponentUpdated"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;
	u8 entityUUID[16];
	std::vector<s8> componentTypeName;
	std::vector<s8> componentName;
	std::vector<u8> componentData;

	inline size_t Size() const
	{
		return 4 + 16*1 + 1 + componentTypeName.size()*1 + 1 + componentName.size()*1 + 4 + componentData.size()*1;
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
		dst.Add<u32>(componentData.size());
		if (componentData.size() > 0)
			dst.AddArray<u8>(&componentData[0], componentData.size());
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
		componentData.resize(src.Read<u32>());
		if (componentData.size() > 0)
			src.ReadArray<u8>(&componentData[0], componentData.size());
	}

};

