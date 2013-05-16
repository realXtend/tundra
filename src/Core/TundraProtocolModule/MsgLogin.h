#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

/// Network message for login request.
struct MsgLogin
{
	MsgLogin()
	{
		InitToDefault();
	}

	MsgLogin(const char *data, size_t numBytes)
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

	enum { messageID = 100 };
	static inline const char * Name() { return "Login"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	std::vector<s8> loginData;

	inline size_t Size() const
	{
		return 2 + loginData.size()*1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u16>((u16)loginData.size());
		if (loginData.size() > 0)
			dst.AddArray<s8>(&loginData[0], (u32)loginData.size());
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		loginData.resize(src.Read<u16>());
		if (loginData.size() > 0)
			src.ReadArray<s8>(&loginData[0], loginData.size());
	}

};

