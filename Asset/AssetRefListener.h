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
    void HandleAssetRefChange(IAttribute *assetRef);

private slots:
    void EmitDownloaded(IAssetTransfer *transfer);

    void EmitDecoded(IAssetTransfer *transfer);

    void EmitLoaded(IAssetTransfer *transfer);

signals:
    /// Emitted when the raw byte download of this asset finishes.
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when a decoder plugin has decoded this asset.
    void Decoded(IAssetTransfer *transfer);

    /// Emitted when this asset is ready to be used in the system.
    void Loaded(IAssetTransfer *transfer);
};

#endif
