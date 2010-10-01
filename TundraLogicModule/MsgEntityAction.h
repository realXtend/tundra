#pragma once

#include "kNet.h"

struct MsgEntityAction
{
	MsgEntityAction()
	{
		InitToDefault();
	}

	MsgEntityAction(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 116; }
	static inline const char *Name() { return "EntityAction"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	struct S_parameters
	{
		std::vector<s8> parameter;

		inline size_t Size() const
		{
			return 1 + parameter.size()*1;
		}

		inline void SerializeTo(kNet::DataSerializer &dst) const
		{
			dst.Add<u8>(parameter.size());
			if (parameter.size() > 0)
				dst.AddArray<s8>(&parameter[0], parameter.size());
		}

		inline void DeserializeFrom(kNet::DataDeserializer &src)
		{
			parameter.resize(src.Read<u8>());
			if (parameter.size() > 0)
				src.ReadArray<s8>(&parameter[0], parameter.size());
		}

	};

	u32 entityId;
	std::vector<s8> name;
	u8 executionType;
	std::vector<S_parameters> parameters;

	inline size_t Size() const
	{
		return 4 + 1 + name.size()*1 + 1 + 1 + kNet::SumArray(parameters, parameters.size());
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u32>(entityId);
		dst.Add<u8>(name.size());
		if (name.size() > 0)
			dst.AddArray<s8>(&name[0], name.size());
		dst.Add<u8>(executionType);
		dst.Add<u8>(parameters.size());
		for(size_t i = 0; i < parameters.size(); ++i)
			parameters[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		entityId = src.Read<u32>();
		name.resize(src.Read<u8>());
		if (name.size() > 0)
			src.ReadArray<s8>(&name[0], name.size());
		executionType = src.Read<u8>();
		parameters.resize(src.Read<u8>());
		for(size_t i = 0; i < parameters.size(); ++i)
			parameters[i].DeserializeFrom(src);
	}

};

