// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetFwd_h
#define incl_Asset_AssetFwd_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class IAsset;
typedef boost::shared_ptr<IAsset> AssetPtr;

class IAssetTransfer;
typedef boost::shared_ptr<IAssetTransfer> AssetTransferPtr;

class IAssetProvider;
typedef boost::shared_ptr<IAssetProvider> AssetProviderPtr;
typedef boost::weak_ptr<IAssetProvider> AssetProviderWeakPtr;

class IAssetStorage;
typedef boost::shared_ptr<IAssetStorage> AssetStoragePtr;

class IAssetUploadTransfer;
typedef boost::shared_ptr<IAssetUploadTransfer> AssetUploadTransferPtr;

struct AssetReference;

class IEventData;

class IAssetTypeFactory;
typedef boost::shared_ptr<IAssetTypeFactory> AssetTypeFactoryPtr;

class AssetRefListener;

namespace Foundation
{
    class Framework;

    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetInterfacePtr;
    
    class ResourceInterface;
    typedef boost::shared_ptr<ResourceInterface> ResourcePtr;
}

#endif
