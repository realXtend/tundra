// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetProvider_h
#define incl_Asset_LocalAssetProvider_h

#include <boost/enable_shared_from_this.hpp>
#include "ThreadTaskManager.h"
#include "AssetModuleApi.h"
#include "IAssetProvider.h"
#include "AssetFwd.h"

namespace HttpUtilities
{
    class HttpTaskResult;
}

namespace Asset
{
    class LocalAssetStorage;

    /// LocalAssetProvider provides Naali scene to use assets from the local file system with 'local://' reference.
    class ASSET_MODULE_API LocalAssetProvider : public QObject, public IAssetProvider, public boost::enable_shared_from_this<LocalAssetProvider>
    {
            Q_OBJECT;

    public:
        explicit LocalAssetProvider(Foundation::Framework* framework);
        
        virtual ~LocalAssetProvider();
        
        //! Returns name of asset provider
        virtual const QString &Name();
        
        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id */
        virtual bool IsValidRef(QString assetRef, QString assetType);
        
        //! Requests an asset for "download"
        /*! \param asset_id Asset UUID
            \param asset_type Asset type
            \param tag Asset request tag, allocated by AssetService
            \return true if asset ID was valid and file could be found (ASSET_READY will be sent in that case) */
        virtual bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag);
        
        virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType);

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
        QString GetPathForAsset(const QString &localFilename);

        //! Asset event category
        event_category_id_t event_category_;
        
        //! Framework
        Foundation::Framework* framework_;
        
        typedef boost::shared_ptr<LocalAssetStorage> LocalAssetStoragePtr;

        //! Asset directories to search, may be recursive or not
        std::vector<LocalAssetStoragePtr> storages;

        //! The following asset uploads are pending to be completed by this provider.
        std::vector<AssetUploadTransferPtr> pendingUploads;

        //! The following asset downloads are pending to be completed by this provider.
        std::vector<AssetTransferPtr> pendingDownloads;

        //! Takes all the pending file download transfers and finishes them.
        void CompletePendingFileDownloads();

        //! Takes all the pending file upload transfers and finishes them.
        void CompletePendingFileUploads();

    private slots:
        void FileChanged(const QString &path);

    };
}


#endif
