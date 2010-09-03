#pragma once

#include "clb/Network/DataDeserializer.h"
#include "clb/Network/DataSerializer.h"

struct MsgClientJoined
{
	MsgClientJoined()
	{
		InitToDefault();
	}

	MsgClientJoined(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 102; }
	static inline const char *Name() { return "ClientJoined"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	u8 userID;
	std::vector<s8> userName;

	inline size_t Size() const
	{
		return 1 + 1 + userName.size()*1;
	}

	inline void SerializeTo(DataSerializer &dst) const
	{
		dst.Add<u8>(userID);
		dst.Add<u8>(userName.size());
		if (userName.size() > 0)
			dst.AddArray<s8>(&userName[0], userName.size());
	}

	inline void DeserializeFrom(DataDeserializer &src)
	{
		userID = src.Read<u8>();
		userName.resize(src.Read<u8>());
		if (userName.size() > 0)
			src.ReadArray<s8>(&userName[0], userName.size());
	}

};

