// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include <QObject>

class IAttribute;

/// Tracks and notifies about asset change events.
class TUNDRACORE_API AssetRefListener : public QObject
{
    Q_OBJECT

public:
    AssetRefListener() : myAssetAPI(0), requestedRef(""), /** \todo This needs to be removed. */ inspectCreated(false) {};

    /// Issues a new asset request to the given AssetReference.
    /// @param assetRef A pointer to an attribute of type AssetReference.
    /// @param assetType Optional asset type name
    void HandleAssetRefChange(IAttribute *assetRef, const QString& assetType = "");

    /// Issues a new asset request to the given assetRef URL.
    /// @param assetApi Pass a pointer to the system Asset API into this function (This utility object doesn't keep reference to framework).
    /// @param assetType Optional asset type name
    void HandleAssetRefChange(AssetAPI *assetApi, QString assetRef, const QString& assetType = "");
    
    /// Returns the asset currently stored in this asset reference.
    AssetPtr Asset() const;

signals:
    /// Emitted when the raw byte download of this asset finishes.
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when this asset is ready to be used in the system.
    void Loaded(AssetPtr asset);

    /// Emitted when the transfer failed
    void TransferFailed(IAssetTransfer *transfer, QString reason);

private slots:
    void OnTransferSucceeded(AssetPtr asset);
    void OnAssetLoaded(AssetPtr asset);
    void OnTransferFailed(IAssetTransfer *transfer, QString reason);
    void OnAssetCreated(AssetPtr asset);

private:
    AssetAPI *myAssetAPI;
    AssetWeakPtr asset;
    AssetTransferWeakPtr currentTransfer;
    AssetReference requestedRef;

    ///\todo This needs to be removed.
    bool inspectCreated;
};
