// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetProvider_h
#define incl_Asset_LocalAssetProvider_h

#include <boost/enable_shared_from_this.hpp>
#include "AssetModuleApi.h"
#include "IAssetProvider.h"
#include "AssetFwd.h"

namespace Asset
{
    class LocalAssetStorage;

    /// LocalAssetProvider provides Naali scene to use assets from the local file system with 'local://' reference.
    class ASSET_MODULE_API LocalAssetProvider : public QObject, public IAssetProvider, public boost::enable_shared_from_this<LocalAssetProvider>
    {
        Q_OBJECT;

    public:
        explicit LocalAssetProvider(Framework* framework);
        
        virtual ~LocalAssetProvider();
        
        /// Returns name of asset provider
        virtual QString Name();
        
        /// Checks an asset id for validity
        /** \return true if this asset provider can handle the id */
        virtual bool IsValidRef(QString assetRef, QString assetType);
                
        virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType);

        /// Performs time-based update 
        /** \param frametime Seconds since last frame */
        virtual void Update(f64 frametime);

        /// Deletes this asset from file.
        virtual void DeleteAssetFromStorage(QString assetRef);

        /// Adds the given directory as an asset storage.
        /** \param directory The paht name for the directory to add.
            \param storageName A human-readable name for the storage. This is used in the UI to the user, but is not an ID of any kind.
            \param recursive If true, all the subfolders of the given folder are added as well. */
        void AddStorageDirectory(const std::string &directory, const std::string &storageName, bool recursive);

        virtual std::vector<AssetStoragePtr> GetStorages() const;

        virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    private:
        typedef boost::shared_ptr<LocalAssetStorage> LocalAssetStoragePtr;

        /// Finds a path where the file localFilename can be found. Searches through all local storages.
        /// @param storage [out] Receives the local storage that contains the asset.
        QString GetPathForAsset(const QString &localFilename, LocalAssetStoragePtr *storage);
        
        /// Framework
        Framework *framework;
        
        /// Asset directories to search, may be recursive or not
        std::vector<LocalAssetStoragePtr> storages;

        /// The following asset uploads are pending to be completed by this provider.
        std::vector<AssetUploadTransferPtr> pendingUploads;

        /// The following asset downloads are pending to be completed by this provider.
        std::vector<AssetTransferPtr> pendingDownloads;

        /// Takes all the pending file download transfers and finishes them.
        void CompletePendingFileDownloads();

        /// Takes all the pending file upload transfers and finishes them.
        void CompletePendingFileUploads();

    private slots:
        void FileChanged(const QString &path);

    };
}


#endif
