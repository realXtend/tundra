// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetStorage_h
#define incl_Asset_HttpAssetStorage_h

#include "AssetAPI.h"
#include "IAssetStorage.h"

class HttpAssetStorage : public IAssetStorage
{
Q_OBJECT

public:
    QString baseAddress;
    QString storageName;

public slots:
    virtual std::vector<IAsset*> GetAllAssets() const { return std::vector<IAsset*>(); }

    /// Specifies whether data can be uploaded to this asset storage.
    virtual bool Writable() const { return false; }

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString &localName) { return GuaranteeTrailingSlash(baseAddress) + localName; }

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return storageName; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return baseAddress; }
};

#endif
