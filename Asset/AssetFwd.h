// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetFwd_h
#define incl_Asset_AssetFwd_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class IAsset;
class IAssetTransfer;
//class IAssetProvider;
class IAssetStorage;
typedef boost::shared_ptr<IAssetStorage> AssetStoragePtr;

class IAssetUploadTransfer;
struct AssetReference;

class IEventData;

class IAssetTypeFactory;
typedef boost::shared_ptr<IAssetTypeFactory> AssetTypeFactoryPtr;

namespace Foundation
{
    class Framework;

    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    class ResourceInterface;
    typedef boost::shared_ptr<ResourceInterface> ResourcePtr;

    class AssetProviderInterface;
    typedef boost::shared_ptr<AssetProviderInterface> AssetProviderPtr;    

    typedef boost::weak_ptr<AssetProviderInterface> AssetProviderWeakPtr;
}

#endif
