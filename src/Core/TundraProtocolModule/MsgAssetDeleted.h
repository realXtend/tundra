#pragma once

#include "kNet/DataDeserializer.h"
#include "kNet/DataSerializer.h"

/// Network message informing that asset has been deleted from storage.
struct MsgAssetDeleted
{
	MsgAssetDeleted()
	{
		InitToDefault();
	}

	MsgAssetDeleted(const char *data, size_t numBytes)
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

	enum { messageID = 122 };
	static inline const char * Name() { return "AssetDeleted"; }

	static const bool defaultReliable = true;
	static const bool defaultInOrder = true;
	static const u32 defaultPriority = 100;

	bool reliable;
	bool inOrder;
	u32 priority;

	std::vector<s8> assetRef;

	inline size_t Size() const
	{
		return 1 + assetRef.size()*1;
	}

	inline void SerializeTo(kNet::DataSerializer &dst) const
	{
		dst.Add<u8>(assetRef.size());
		if (assetRef.size() > 0)
			dst.AddArray<s8>(&assetRef[0], assetRef.size());
	}

	inline void DeserializeFrom(kNet::DataDeserializer &src)
	{
		assetRef.resize(src.Read<u8>());
		if (assetRef.size() > 0)
			src.ReadArray<s8>(&assetRef[0], assetRef.size());
	}

};

