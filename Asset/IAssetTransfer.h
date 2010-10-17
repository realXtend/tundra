// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetTransfer_h
#define incl_Asset_IAssetTransfer_h

#include <QObject>

#include "AssetFwd.h"

class IAssetTransfer : public QObject
{
    Q_OBJECT

public:
    virtual ~IAssetTransfer() {}

    /// Points to the actual asset if it has been loaded in.
    IAsset *asset;

    /// Points to the actual asset if it has been loaded in. This member is implemented for legacy purposes to help 
    /// transition period to new Asset API. Will be removed. -jj
    Foundation::AssetPtr assetPtr;

    void EmitAssetDownloaded();

    void EmitAssetDecoded();

    void EmitAssetLoaded();

signals:
    /// Emitted when the raw byte download of this asset finishes.
    void Downloaded();

    /// Emitted when a decoder plugin has decoded this asset.
    void Decoded();

    /// Emitted when this asset is ready to be used in the system.
    void Loaded();
};

//class LocalAssetTransfer : public IAssetTransfer;
//class HttpAssetTransfer : public IAssetTransfer;
//class KNetAssetTransfer : public IAssetProvider;

#endif
