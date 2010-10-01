#pragma once

#include "kNet.h"

struct MsgRemoveComponents
{
	MsgRemoveComponents()
	{
		InitToDefault();
	}

	MsgRemoveComponents(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 114; }
	static inline const char *Name() { return "RemoveComponents"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	struct S_components
	{
		u32 componentTypeHash;
		std::vector<s8> componentName;

		inline size_t Size() const
		{
			return 4 + 1 + componentName.size()*1;
		}

		inline void SerializeTo(kNet::DataSerializer &dst) const
		{
			dst.Add<u32>(componentTypeHash);
			dst.Add<u8>(componentName.size());
			if (componentName.size() > 0)
				dst.AddArray<s8>(&componentName[0], componentName.size());
		}

		inline void DeserializeFrom(kNet::DataDeserializer &src)
		{
			componentTypeHash = src.Read<u32>();
			componentName.resize(src.Read<u8>());
			if (componentName.size() > 0)
				src.ReadArray<s8>(&componentName[0], componentName.size());
		}

	};

	u32 entityID;
	std::vector<S_components> components;

	inline size_t Size() const
	{
		return 4 + 1 + kNet::SumArray(components, components.size());
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.Add<u8>(components.size());
		for(size_t i = 0; i < components.size(); ++i)
			components[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		components.resize(src.Read<u8>());
		for(size_t i = 0; i < components.size(); ++i)
			components[i].DeserializeFrom(src);
	}

};

