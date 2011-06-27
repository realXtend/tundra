#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

struct MsgLoginReply
{
	MsgLoginReply()
	{
		InitToDefault();
	}

	MsgLoginReply(const char *data, size_t numBytes)
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

	enum { messageID = 101 };
	static inline const char * const Name() { return "LoginReply"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	u8 success;
	u8 userID;
	std::vector<s8> loginReplyData;

	inline size_t Size() const
	{
		return 1 + 1 + 2 + loginReplyData.size()*1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u8>(success);
		dst.Add<u8>(userID);
		dst.Add<u16>(loginReplyData.size());
		if (loginReplyData.size() > 0)
			dst.AddArray<s8>(&loginReplyData[0], loginReplyData.size());
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		success = src.Read<u8>();
		userID = src.Read<u8>();
		loginReplyData.resize(src.Read<u16>());
		if (loginReplyData.size() > 0)
			src.ReadArray<s8>(&loginReplyData[0], loginReplyData.size());
	}

};

