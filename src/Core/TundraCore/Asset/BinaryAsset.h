// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "AssetAPI.h"
#include "IAsset.h"

/// A generic data container for assets of unknown type.
class TUNDRACORE_API BinaryAsset : public IAsset
{
    Q_OBJECT

public:
    BinaryAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
        IAsset(owner, type_, name_)
    {
    }

    ~BinaryAsset()
    {
        Unload();
    }

    virtual void DoUnload()
    {
        data.clear();
    }

    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, bool /*allowAsynchronous*/)
    {
        data.clear();
        data.insert(data.end(), data_, data_ + numBytes);
        assetAPI->AssetLoadCompleted(Name());
        return true;
    }

    virtual bool SerializeTo(std::vector<u8> &dst, const QString &/*serializationParameters*/) const
    {
        dst = data;
        return true;
    }

    virtual std::vector<AssetReference> FindReferences() const
    {
        return std::vector<AssetReference>();
    }

    bool IsLoaded() const
    {
        return data.size() > 0;
    }

    std::vector<u8> data;
};
