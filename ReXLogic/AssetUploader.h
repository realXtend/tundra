// For conditions of distribution and use, see copyright notice in license.txt

/// @file AssetUploader.h
/// @brief Asset HTTP uploading functionality for OS & reX servers.

#ifndef incl_AssetUploader_h
#define incl_AssetUploader_h

#include "RexCommon.h"
#include "RexUUID.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class InventorySkeleton;
}

namespace RexLogic
{
    class AssetUploader
    {
    public:
        /// Default constructor.
        AssetUploader(Foundation::Framework* framework);

        /// Destructor.
        virtual ~AssetUploader();

        /// @return Does asset uploader have an upload capability.
        bool HasUploadCapability() const { return uploadCapability_ != ""; }

        /// Sets the upload capability URL.
        /// @param url Capability URL.
        void SetUploadCapability(const std::string &url) { uploadCapability_ = url; }

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
            const RexTypes::RexUUID &folder_id);

        /// Uploads multiple files using HTTP.
        /// @param files List of AssetInfo structs.
        /// @param inventory Pointer to the user's inventory.
        void UploadFiles(Core::StringList filenames, OpenSimProtocol::InventorySkeleton *inventory);

        /// Utility function for create name for asset from filename.
        /// @param filename Filename.
        /// @return Filename without the file extension.
        std::string CreateNameFromFilename(std::string filename);

    private:
        /// Framework pointer. Needed for EventManager.
        Foundation::Framework* framework_;

        /// Creates NewFileAgentInventory XML message.
        std::string CreateNewFileAgentInventoryXML(
            const std::string &asset_type,
            const std::string &inventory_type,
            const std::string &folder_id,
            const std::string &name,
            const std::string &description);

        /// Upload capability URL.
        std::string uploadCapability_;
    };
}

#endif
