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

	struct S_permissions
	{
		u8 userUUID[16];
		u32 permissionFlags;

		inline size_t Size() const
		{
			return 16*1 + 4;
		}

		inline void SerializeTo(DataSerializer &dst) const
		{
			dst.AddArray<u8>(userUUID, 16);
			dst.Add<u32>(permissionFlags);
		}

		inline void DeserializeFrom(DataDeserializer &src)
		{
			src.ReadArray<u8>(userUUID, 16);
			permissionFlags = src.Read<u32>();
		}

	};

	struct S_initialComponents
	{
		std::vector<s8> componentTypeName;
		std::vector<s8> componentName;
		std::vector<u8> componentData;

		inline size_t Size() const
		{
			return 1 + componentTypeName.size()*1 + 1 + componentName.size()*1 + 4 + componentData.size()*1;
		}

		inline void SerializeTo(DataSerializer &dst) const
		{
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

	u32 actionID;
	std::vector<S_permissions> permissions;
	std::vector<S_initialComponents> initialComponents;

	inline size_t Size() const
	{
		return 4 + 1 + SumArray(permissions, permissions.size()) + 1 + SumArray(initialComponents, initialComponents.size());
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(actionID);
		dst.Add<u8>(permissions.size());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].SerializeTo(dst);
		dst.Add<u8>(initialComponents.size());
		for(size_t i = 0; i < initialComponents.size(); ++i)
			initialComponents[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		actionID = src.Read<u32>();
		permissions.resize(src.Read<u8>());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].DeserializeFrom(src);
		initialComponents.resize(src.Read<u8>());
		for(size_t i = 0; i < initialComponents.size(); ++i)
			initialComponents[i].DeserializeFrom(src);
	}

};

