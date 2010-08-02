#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgServerReset
{
	MsgServerReset()
	{
		InitToDefault();
	}

	MsgServerReset(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 107; }
	static inline const char *Name() { return "ServerReset"; }

	bool reliable;
	bool inOrder;
	u32 priority;


	inline size_t Size() const
	{
		return 0;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
	}

};

