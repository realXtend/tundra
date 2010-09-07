#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgLogout
{
	MsgLogout()
	{
		InitToDefault();
	}

	MsgLogout(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 104; }
	static inline const char *Name() { return "Logout"; }

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

