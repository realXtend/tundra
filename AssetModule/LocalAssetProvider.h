// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetProvider_h
#define incl_Asset_LocalAssetProvider_h

#include <boost/enable_shared_from_this.hpp>
#include "ThreadTaskManager.h"
#include "AssetModuleApi.h"
#include "AssetProviderInterface.h"
#include "AssetFwd.h"

namespace HttpUtilities
{
    class HttpTaskResult;
}

namespace Asset
{
    class LocalAssetStorage;

    /// Adds a trailing slash to the given string representing a directory path if it doesn't have one at the end already.
    QString GuaranteeTrailingSlash(const QString &source);

    /// LocalAssetProvider provides Naali scene to use assets from the local file system with 'file://' reference.
    class ASSET_MODULE_API LocalAssetProvider : public QObject, public Foundation::AssetProviderInterface, public boost::enable_shared_from_this<LocalAssetProvider>
    {
            Q_OBJECT;

    public:
        explicit LocalAssetProvider(Foundation::Framework* framework);
        
        virtual ~LocalAssetProvider();
        
        //! Returns name of asset provider
        virtual const std::string& Name();
        
        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id */
        virtual bool IsValidId(const std::string& asset_id, const std::string& asset_type);
        
        //! Requests an asset for "download"
        /*! \param asset_id Asset UUID
            \param asset_type Asset type
            \param tag Asset request tag, allocated by AssetService
            \return true if asset ID was valid and file could be found (ASSET_READY will be sent in that case) */
        virtual bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag);
        
//        virtual IAssetTransfer *RequestAsset(QString assetRef);

        //! Returns whether a certain asset is already being "downloaded". Returns always false.
        virtual bool InProgress(const std::string& asset_id);
        
        //! Queries status of asset "download"
        /*! \param asset_id Asset UUID
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return false will always be returned (not supported) */
        virtual bool QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous);
        
        //! Gets incomplete asset
        /*! If transfer not in progress or not enough bytes received, will return empty pointer
            
            \param asset_id Asset UUID
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Null pointer will always be returned (not supported) */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received);   
        
        //! Returns information about current asset transfers
        virtual Foundation::AssetTransferInfoVector GetTransferInfo() { return Foundation::AssetTransferInfoVector(); }
        
        //! Performs time-based update 
        /*! \param frametime Seconds since last frame */
        virtual void Update(f64 frametime);
                
        //! Adds the given directory as an asset storage.
        /*! \param directory The paht name for the directory to add.
            \param storageName A human-readable name for the storage. This is used in the UI to the user, but is not an ID of any kind.
            \param recursive If true, all the subfolders of the given folder are added as well. */
        void AddStorageDirectory(const std::string &directory, const std::string &storageName, bool recursive);

        virtual std::vector<AssetStoragePtr> GetStorages() const;

        virtual IAssetUploadTransfer *UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName);

        virtual IAssetUploadTransfer *UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    private:

        //! Get a path for asset, using all the search directories
        std::string GetPathForAsset(const std::string& assetname);

        //! Asset event category
        event_category_id_t event_category_;
        
        //! Framework
        Foundation::Framework* framework_;
        
        typedef boost::shared_ptr<LocalAssetStorage> LocalAssetStoragePtr;

        //! Asset directories to search, may be recursive or not
        std::vector<LocalAssetStoragePtr> storages;

        typedef boost::shared_ptr<IAssetUploadTransfer> AssetUploadTransferPtr;

        //! The following asset uploads are pending to be completed by this provider.
        std::vector<AssetUploadTransferPtr > pendingUploads;

        //! Takes all the pending file upload transfers and finishes them.
        void CompletePendingFileUploads();

    private slots:
        void FileChanged(const QString &path);

    };
}


#endif
