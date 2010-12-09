// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetTransfer_h
#define incl_Asset_IAssetTransfer_h

#include <boost/enable_shared_from_this.hpp>
#include <QObject>
#include <vector>

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

class IAssetTransfer : public QObject, public boost::enable_shared_from_this<IAssetTransfer>
{
    Q_OBJECT

public:
    virtual ~IAssetTransfer() {}

    /// Points to the actual asset if it has been loaded in.
    AssetPtr asset;

    /// Specifies the full source URL where this asset was loaded from.
    AssetReference source;

    /// Specifies the actual type of the asset.
    QString assetType;

    /// Specifies the internal resource name. Will not be filled until the resource is loaded.
    QString internalResourceName;

    /// Specifies the provider this asset is being downloaded from.
    AssetProviderWeakPtr provider;

    /// Specifies the storage this asset is being downloaded from.
    AssetStorageWeakPtr storage;

    /// Points to the actual asset if it has been loaded in. This member is implemented for legacy purposes to help 
    /// transition period to new Asset API. Will be removed. -jj
//    Foundation::AssetInterfacePtr assetPtr;
//    Foundation::ResourcePtr resourcePtr;

    void EmitAssetDownloaded();

    void EmitAssetDecoded();

    void EmitAssetLoaded();

    /// Stores the raw asset bytes for this asset.
    std::vector<u8> rawAssetData;

    /// Returns the current transfer progress in the range [0, 1].
    // float Progress() const;

signals:
    /// Emitted when the raw byte download of this asset finishes.
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when a decoder plugin has decoded this asset.
    void Decoded(IAssetTransfer *transfer);

    /// Emitted when this asset is ready to be used in the system.
    void Loaded(IAssetTransfer *transfer);
};

//class LocalAssetTransfer : public IAssetTransfer;
//class HttpAssetTransfer : public IAssetTransfer;
//class KNetAssetTransfer : public IAssetProvider;

#endif
