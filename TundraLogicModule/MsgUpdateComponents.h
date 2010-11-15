#pragma once

#include "kNet.h"

struct MsgUpdateComponents
{
	MsgUpdateComponents()
	{
		InitToDefault();
	}

	MsgUpdateComponents(const char *data, size_t numBytes)
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

	static inline u32 MessageID() { return 113; }
	static inline const char *Name() { return "UpdateComponents"; }

	bool reliable;
	bool inOrder;
	u32 priority;

	struct S_components
	{
		u32 componentTypeHash;
		std::vector<s8> componentName;
		std::vector<u8> componentData;

		inline size_t Size() const
		{
			return 4 + 1 + componentName.size()*1 + 2 + componentData.size()*1;
		}

		inline void SerializeTo(kNet::DataSerializer &dst) const
		{
			dst.Add<u32>(componentTypeHash);
			dst.Add<u8>(componentName.size());
			if (componentName.size() > 0)
				dst.AddArray<s8>(&componentName[0], componentName.size());
			dst.Add<u16>(componentData.size());
			if (componentData.size() > 0)
				dst.AddArray<u8>(&componentData[0], componentData.size());
		}

		inline void DeserializeFrom(kNet::DataDeserializer &src)
		{
			componentTypeHash = src.Read<u32>();
			componentName.resize(src.Read<u8>());
			if (componentName.size() > 0)
				src.ReadArray<s8>(&componentName[0], componentName.size());
			componentData.resize(src.Read<u16>());
			if (componentData.size() > 0)
				src.ReadArray<u8>(&componentData[0], componentData.size());
		}

	};

	struct S_dynamiccomponents
	{
		struct S_attributes
		{
			std::vector<s8> attributeName;
			std::vector<s8> attributeType;
			std::vector<u8> attributeData;

			inline size_t Size() const
			{
				return 1 + attributeName.size()*1 + 1 + attributeType.size()*1 + 2 + attributeData.size()*1;
			}

			inline void SerializeTo(kNet::DataSerializer &dst) const
			{
				dst.Add<u8>(attributeName.size());
				if (attributeName.size() > 0)
					dst.AddArray<s8>(&attributeName[0], attributeName.size());
				dst.Add<u8>(attributeType.size());
				if (attributeType.size() > 0)
					dst.AddArray<s8>(&attributeType[0], attributeType.size());
				dst.Add<u16>(attributeData.size());
				if (attributeData.size() > 0)
					dst.AddArray<u8>(&attributeData[0], attributeData.size());
			}

			inline void DeserializeFrom(kNet::DataDeserializer &src)
			{
				attributeName.resize(src.Read<u8>());
				if (attributeName.size() > 0)
					src.ReadArray<s8>(&attributeName[0], attributeName.size());
				attributeType.resize(src.Read<u8>());
				if (attributeType.size() > 0)
					src.ReadArray<s8>(&attributeType[0], attributeType.size());
				attributeData.resize(src.Read<u16>());
				if (attributeData.size() > 0)
					src.ReadArray<u8>(&attributeData[0], attributeData.size());
			}

		};

		u32 componentTypeHash;
		std::vector<s8> componentName;
		std::vector<S_attributes> attributes;

		inline size_t Size() const
		{
			return 4 + 1 + componentName.size()*1 + 1 + kNet::SumArray(attributes, attributes.size());
		}

		inline void SerializeTo(kNet::DataSerializer &dst) const
		{
			dst.Add<u32>(componentTypeHash);
			dst.Add<u8>(componentName.size());
			if (componentName.size() > 0)
				dst.AddArray<s8>(&componentName[0], componentName.size());
			dst.Add<u8>(attributes.size());
			for(size_t i = 0; i < attributes.size(); ++i)
				attributes[i].SerializeTo(dst);
		}

		inline void DeserializeFrom(kNet::DataDeserializer &src)
		{
			componentTypeHash = src.Read<u32>();
			componentName.resize(src.Read<u8>());
			if (componentName.size() > 0)
				src.ReadArray<s8>(&componentName[0], componentName.size());
			attributes.resize(src.Read<u8>());
			for(size_t i = 0; i < attributes.size(); ++i)
				attributes[i].DeserializeFrom(src);
		}

	};

	u32 entityID;
	std::vector<S_components> components;
	std::vector<S_dynamiccomponents> dynamiccomponents;

	inline size_t Size() const
	{
		return 4 + 1 + kNet::SumArray(components, components.size()) + 1 + kNet::SumArray(dynamiccomponents, dynamiccomponents.size());
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u32>(entityID);
		dst.Add<u8>(components.size());
		for(size_t i = 0; i < components.size(); ++i)
			components[i].SerializeTo(dst);
		dst.Add<u8>(dynamiccomponents.size());
		for(size_t i = 0; i < dynamiccomponents.size(); ++i)
			dynamiccomponents[i].SerializeTo(dst);
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		entityID = src.Read<u32>();
		components.resize(src.Read<u8>());
		for(size_t i = 0; i < components.size(); ++i)
			components[i].DeserializeFrom(src);
		dynamiccomponents.resize(src.Read<u8>());
		for(size_t i = 0; i < dynamiccomponents.size(); ++i)
			dynamiccomponents[i].DeserializeFrom(src);
	}

};

