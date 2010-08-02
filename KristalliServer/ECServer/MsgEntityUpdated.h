#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgEntityUpdated
{
	MsgEntityUpdated()
	{
		InitToDefault();
	}

	MsgEntityUpdated(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 120; }
	static inline const char *Name() { return "EntityUpdated"; }

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

	u32 entityID;
	u8 entityUUID[16];
	u8 creatorUUID[16];
	std::vector<S_permissions> permissions;

	inline size_t Size() const
	{
		return 4 + 16*1 + 16*1 + 1 + SumArray(permissions, permissions.size());
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.AddArray<u8>(entityUUID, 16);
		dst.AddArray<u8>(creatorUUID, 16);
		dst.Add<u8>(permissions.size());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		src.ReadArray<u8>(entityUUID, 16);
		src.ReadArray<u8>(creatorUUID, 16);
		permissions.resize(src.Read<u8>());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].DeserializeFrom(src);
	}

};

