// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetRefListener_h
#define incl_Asset_AssetRefListener_h

#include <QObject>
#include "AssetFwd.h"

class IAttribute;

class AssetRefListener : public QObject
{
    Q_OBJECT;

public:
    /// Issues a new asset request to the given AssetReference.
    /// @param assetRef A pointer to an attribute of type AssetReference.
    /// @param assetType Optional asset type name
    void HandleAssetRefChange(IAttribute *assetRef, const QString& assetType = "");

    /// Issues a new asset request to the given assetRef URL.
    /// @param assetApi Pass a pointer to the system Asset API into this function (This utility object doesn't keep reference to framework).
    /// @param assetType Optional asset type name
    void HandleAssetRefChange(AssetAPI *assetApi, QString assetRef, const QString& assetType = "");
    
    /// Returns the asset currently stored in this asset reference.
    AssetPtr Asset();

signals:
    /// Emitted when the raw byte download of this asset finishes.
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when a decoder plugin has decoded this asset.
    void Decoded(AssetPtr asset);

    /// Emitted when this asset is ready to be used in the system.
    void Loaded(AssetPtr asset);

private slots:
    void EmitDownloaded(IAssetTransfer *transfer);

    void EmitDecoded(AssetPtr asset);

    void EmitLoaded(AssetPtr asset);

private:
    AssetWeakPtr asset;
};

#endif
