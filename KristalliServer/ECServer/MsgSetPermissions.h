#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgSetPermissions
{
	MsgSetPermissions()
	{
		InitToDefault();
	}

	MsgSetPermissions(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 114; }
	static inline const char *Name() { return "SetPermissions"; }

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

	u32 actionID;
	u32 entityID;
	u8 entityUUID[16];
	std::vector<S_permissions> permissions;

	inline size_t Size() const
	{
		return 4 + 4 + 16*1 + 1 + SumArray(permissions, permissions.size());
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u32>(actionID);
		dst.Add<u32>(entityID);
		dst.AddArray<u8>(entityUUID, 16);
		dst.Add<u8>(permissions.size());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		actionID = src.Read<u32>();
		entityID = src.Read<u32>();
		src.ReadArray<u8>(entityUUID, 16);
		permissions.resize(src.Read<u8>());
		for(size_t i = 0; i < permissions.size(); ++i)
			permissions[i].DeserializeFrom(src);
	}

};

