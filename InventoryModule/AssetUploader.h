// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.h
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#ifndef incl_InventoryModule_AssetUploader_h
#define incl__InventoryModule_AssetUploader_h

#include "WorldStream.h"

#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace Inventory
{
    class OpenSimInventoryDataModel;

    class AssetUploader : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework pointer.
        /// @param data_model OpenSim inventory data model pointer.
        AssetUploader(Foundation::Framework* framework, OpenSimInventoryDataModel *data_model);

        /// Destructor.
        ~AssetUploader();

        /// Sets world stream pointer.
        void SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream);

    public slots:
        /// Uploads multiple files using HTTP.
        /// @param filenames list of filenames.
        void UploadFiles(StringList &filenames);
        void UploadFiles(QStringList &filenames);

        /// Uploads multiple assets using HTTP.
        /// @param files List of filenames (determines asset type & folder to use)
        /// @param data List of asset data buffers
        void UploadBuffers(QStringList &filenames, QVector<QVector<uchar> > &buffers);

        /** Uploads a file using HTTP.
         *  @param asset_type_t Asset type.
         *  @param filename Filename.
         *  @param name User-defined name.
         *  @param description User-defined description.
         *  @param folder_id Id of the destination folder for this item.
         *  @return true if successful
         */
        bool UploadFile(
            const RexTypes::asset_type_t &asset_type,
            const std::string &filename,
            const std::string &name,
            const std::string &description,
            const RexUUID &folder_id);

        /** Uploads a buffer using HTTP.
         *  @param asset_type_t Asset type.
         *  @param filename Filename (used to decide asset type)
         *  @param name User-defined name.
         *  @param description User-defined description.
         *  @param folder_id Id of the destination folder for this item.
         *  @param data buffer
         *  @return true if successful
         */
        bool UploadBuffer(
            const RexTypes::asset_type_t &asset_type,
            const std::string &filename,
            const std::string &name,
            const std::string &description,
            const RexUUID &folder_id,
            const QVector<uchar>& buffer);

        /// @return Does asset uploader have upload capability set.
        bool HasUploadCapability() const { return uploadCapability_ != ""; }

        /// Sets the upload capability URL.
        /// @param url Capability URL.
        void SetUploadCapability(const std::string &url) { uploadCapability_ = url; }

        /// Utility function for create name for asset from filename.
        /// @param filename Filename.
        /// @return Filename without the file extension.
        std::string CreateNameFromFilename(std::string filename);

    private:
        Q_DISABLE_COPY(AssetUploader);

        ///
        /*
        bool ThreadedUploadFile(
            const RexTypes::asset_type_t &asset_type,
            const std::string &filename,
            const std::string &name,
            const std::string &description,
            const RexUUID &folder_id);
        */

        /// Used by UploadFiles.
        void ThreadedUploadFiles(StringList filenames);

        /// Used by UploadBuffers.
        //void ThreadedUploadBuffers(StringList filenames, std::vector<std::vector<u8> > buffers);
        void ThreadedUploadBuffers(QStringList filenames, QVector<QVector<uchar> > buffers);

        /// Creates NewFileAgentInventory XML message.
        std::string CreateNewFileAgentInventoryXML(
            const std::string &asset_type,
            const std::string &inventory_type,
            const std::string &folder_id,
            const std::string &name,
            const std::string &description);

        /// Creates all the reX-spesific asset folders to the inventory.
        void CreateRexInventoryFolders();

        /// Framework pointer. Needed for EventManager.
        Foundation::Framework* framework_;

        /// Upload capability URL.
        std::string uploadCapability_;

        /// WorldStream pointer
        ProtocolUtilities::WorldStreamPtr currentWorldStream_;

        /// Inventory data model pointer.
        OpenSimInventoryDataModel *dataModel_;
    };
}

#endif
