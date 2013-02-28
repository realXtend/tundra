// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"

class Framework;
class AssetAPI;
class AssetCache;

class IAsset;
typedef shared_ptr<IAsset> AssetPtr;
typedef weak_ptr<IAsset> AssetWeakPtr;

class IAssetBundle;
typedef shared_ptr<IAssetBundle> AssetBundlePtr;
typedef weak_ptr<IAssetBundle> AssetBundleWeakPtr;

class IAssetTransfer;
typedef shared_ptr<IAssetTransfer> AssetTransferPtr;
typedef weak_ptr<IAssetTransfer> AssetTransferWeakPtr;

class AssetBundleMonitor;
typedef shared_ptr<AssetBundleMonitor> AssetBundleMonitorPtr;
typedef weak_ptr<AssetBundleMonitor> AssetBundleMonitorWeakPtr;
struct SubAssetLoader;

class IAssetProvider;
typedef shared_ptr<IAssetProvider> AssetProviderPtr;
typedef weak_ptr<IAssetProvider> AssetProviderWeakPtr;

class IAssetStorage;
typedef shared_ptr<IAssetStorage> AssetStoragePtr;
typedef weak_ptr<IAssetStorage> AssetStorageWeakPtr;

class IAssetUploadTransfer;
typedef shared_ptr<IAssetUploadTransfer> AssetUploadTransferPtr;

struct AssetReference;
struct AssetReferenceList;

class IAssetTypeFactory;
typedef shared_ptr<IAssetTypeFactory> AssetTypeFactoryPtr;

class IAssetBundleTypeFactory;
typedef shared_ptr<IAssetBundleTypeFactory> AssetBundleTypeFactoryPtr;

class AssetRefListener;
typedef shared_ptr<AssetRefListener> AssetRefListenerPtr;

class Framework;
class BinaryAsset;
typedef shared_ptr<BinaryAsset> BinaryAssetPtr;
