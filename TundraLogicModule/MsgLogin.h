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

	std::vector<s8> userName;
	std::vector<s8> password;

	inline size_t Size() const
	{
		return 1 + userName.size()*1 + 1 + password.size()*1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u8>(userName.size());
		if (userName.size() > 0)
			dst.AddArray<s8>(&userName[0], userName.size());
		dst.Add<u8>(password.size());
		if (password.size() > 0)
			dst.AddArray<s8>(&password[0], password.size());
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		userName.resize(src.Read<u8>());
		if (userName.size() > 0)
			src.ReadArray<s8>(&userName[0], userName.size());
		password.resize(src.Read<u8>());
		if (password.size() > 0)
			src.ReadArray<s8>(&password[0], password.size());
	}

};

