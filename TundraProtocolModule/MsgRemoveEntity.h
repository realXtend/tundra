#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

struct MsgRemoveEntity
{
	MsgRemoveEntity()
	{
		InitToDefault();
	}

	MsgRemoveEntity(const char *data, size_t numBytes)
	{
		InitToDefault();
		kNet::DataDeserializer dd(data, numBytes);
		DeserializeFrom(dd);
	}

	void InitToDefault()
	{
		reliable = defaultReliable;
		inOrder = defaultInOrder;
		priority = defaultPriority;
	}

	enum { messageID = 111 };
	static inline const char * const Name() { return "RemoveEntity"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	u32 entityID;

	inline size_t Size() const
	{
		return 4;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		entityID = src.Read<u32>();
	}

};

