// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetStorage_h
#define incl_Asset_AssetStorage_h

#include <QObject>
#include <QString>
#include <vector>
#include "AssetFwd.h"

class IAssetStorage : public QObject
{
    Q_OBJECT
public:

    virtual ~IAssetStorage() {}

    /// Returns all assets saved in this asset storage.
    std::vector<IAsset*> GetAllAssets() const;

    /// Starts a new asset upload to this storage. If the given asset exists already in the storage, it is replaced.
    /// @param url The desired name for the asset.
    /// @return A pointer to the newly created transfer.
    IAssetTransfer *UploadAsset(const char *data, size_t numBytes, QString url);

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    QString Name() const;

    /// Returns the address of this storage.
    QString BaseURL() const;
};

#endif
