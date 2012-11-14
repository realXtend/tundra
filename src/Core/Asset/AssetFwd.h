// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Framework;
class AssetAPI;
class AssetCache;

class IAsset;
typedef boost::shared_ptr<IAsset> AssetPtr;
typedef boost::weak_ptr<IAsset> AssetWeakPtr;

class IAssetBundle;
typedef boost::shared_ptr<IAssetBundle> AssetBundlePtr;
typedef boost::weak_ptr<IAssetBundle> AssetBundleWeakPtr;

class IAssetTransfer;
typedef boost::shared_ptr<IAssetTransfer> AssetTransferPtr;
typedef boost::weak_ptr<IAssetTransfer> AssetTransferWeakPtr;

class AssetBundleMonitor;
typedef boost::shared_ptr<AssetBundleMonitor> AssetBundleMonitorPtr;
typedef boost::weak_ptr<AssetBundleMonitor> AssetBundleMonitorWeakPtr;
struct SubAssetLoader;

class IAssetProvider;
typedef boost::shared_ptr<IAssetProvider> AssetProviderPtr;
typedef boost::weak_ptr<IAssetProvider> AssetProviderWeakPtr;

class IAssetStorage;
typedef boost::shared_ptr<IAssetStorage> AssetStoragePtr;
typedef boost::weak_ptr<IAssetStorage> AssetStorageWeakPtr;

class IAssetUploadTransfer;
typedef boost::shared_ptr<IAssetUploadTransfer> AssetUploadTransferPtr;

struct AssetReference;
struct AssetReferenceList;

class IAssetTypeFactory;
typedef boost::shared_ptr<IAssetTypeFactory> AssetTypeFactoryPtr;

class IAssetBundleTypeFactory;
typedef boost::shared_ptr<IAssetBundleTypeFactory> AssetBundleTypeFactoryPtr;

class AssetRefListener;
typedef boost::shared_ptr<AssetRefListener> AssetRefListenerPtr;

class Framework;
class BinaryAsset;
typedef boost::shared_ptr<BinaryAsset> BinaryAssetPtr;
