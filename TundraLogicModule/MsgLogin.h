#pragma once

#include "kNet.h"

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
		reliable = true;
		inOrder = true;
		priority = 100;
	}

	static inline u32 MessageID() { return 100; }
	static inline const char *Name() { return "Login"; }

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
		dst.Add<u16>(loginData.size());
		if (loginData.size() > 0)
			dst.AddArray<s8>(&loginData[0], loginData.size());
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		loginData.resize(src.Read<u16>());
		if (loginData.size() > 0)
			src.ReadArray<s8>(&loginData[0], loginData.size());
	}

};

