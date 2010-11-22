// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetTransfer_h
#define incl_Asset_IAssetTransfer_h

#include <QObject>

#include "AssetFwd.h"
#include "AssetReference.h"

class IAssetTransfer : public QObject
{
    Q_OBJECT

public:
    virtual ~IAssetTransfer() {}

    /// Points to the actual asset if it has been loaded in.
    boost::shared_ptr<IAsset> asset;

    /// Specifies the source where this asset was loaded from.
    AssetReference source;

    /// Specifies the internal resource name. Will not be filled until the resource is loaded
    QString internalResourceName;
    
    /// Points to the actual asset if it has been loaded in. This member is implemented for legacy purposes to help 
    /// transition period to new Asset API. Will be removed. -jj
    Foundation::AssetPtr assetPtr;
    Foundation::ResourcePtr resourcePtr;

    void EmitAssetDownloaded();

    void EmitAssetDecoded();

    void EmitAssetLoaded();

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
