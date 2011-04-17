// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetFwd_h
#define incl_Asset_AssetFwd_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class AssetAPI;
class AssetCache;

class IAsset;
typedef boost::shared_ptr<IAsset> AssetPtr;
typedef boost::weak_ptr<IAsset> AssetWeakPtr;

class IAssetTransfer;
typedef boost::shared_ptr<IAssetTransfer> AssetTransferPtr;
typedef boost::weak_ptr<IAssetTransfer> AssetTransferWeakPtr;

class IAssetProvider;
typedef boost::shared_ptr<IAssetProvider> AssetProviderPtr;
typedef boost::weak_ptr<IAssetProvider> AssetProviderWeakPtr;

class IAssetStorage;
typedef boost::shared_ptr<IAssetStorage> AssetStoragePtr;
typedef boost::weak_ptr<IAssetStorage> AssetStorageWeakPtr;

class IAssetUploadTransfer;
typedef boost::shared_ptr<IAssetUploadTransfer> AssetUploadTransferPtr;

struct AssetReference;

class IAssetTypeFactory;
typedef boost::shared_ptr<IAssetTypeFactory> AssetTypeFactoryPtr;

class AssetRefListener;
typedef boost::shared_ptr<AssetRefListener> AssetRefListenerPtr;

namespace Foundation
{
    class Framework;
}

#endif
