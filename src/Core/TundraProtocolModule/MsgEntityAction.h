#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

/// Network message for entity-action replication.
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
		reliable = defaultReliable;
		inOrder = defaultInOrder;
		priority = defaultPriority;
	}

	enum { messageID = 120 };
	static inline const char * Name() { return "EntityAction"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	struct S_parameters
	{
		std::vector<s8> parameter;

		inline size_t Size() const
		{
            // This function has been manually modified, and not generated using the MessageCompiler tool.
            // kNet does not support setting VLE fields as dynamicCount length fields.
            return (size_t)kNet::VLE8_16_32::GetEncodedBitLength(parameter.size()) / 8 + parameter.size()*1;
		}

		inline void SerializeTo(kNet::DataSerializer &dst) const
		{
            // This function has been manually modified, and not generated using the MessageCompiler tool.
            // kNet does not support setting VLE fields as dynamicCount length fields.
			dst.AddVLE<kNet::VLE8_16_32>(parameter.size());
			if (parameter.size() > 0)
				dst.AddArray<s8>(&parameter[0], parameter.size());
		}

		inline void DeserializeFrom(kNet::DataDeserializer &src)
		{
            // This function has been manually modified, and not generated using the MessageCompiler tool.
            // kNet does not support setting VLE fields as dynamicCount length fields.
			parameter.resize(src.ReadVLE<kNet::VLE8_16_32>());
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
		return 4 + 1 + name.size()*1 + 1 + 1 + kNet::ArraySize<kNet::TypeSerializer<S_parameters> >(parameters, parameters.size());
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
			kNet::TypeSerializer<S_parameters>::SerializeTo(dst, parameters[i]);
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
			kNet::TypeSerializer<S_parameters>::DeserializeFrom(src, parameters[i]);
	}

};

