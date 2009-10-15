// For conditions of distribution and use, see copyright notice in license.txt

/// @file Inventory.h
/// @brief Inventory structure skeleton .

#ifndef incl_OSProto_Inventory_h
#define incl_OSProto_Inventory_h

#include "OpenSimProtocolModuleApi.h"
#include "RexUUID.h"

namespace OpenSimProtocol
{
    using namespace RexTypes;

    class OSPROTO_MODULE_API InventoryAssetSkeleton
    {
    public:
        /// Default constructor.
        InventoryAssetSkeleton() {}

        /// Destructor.
        virtual ~InventoryAssetSkeleton() {}

        /// Inventory item ID.
        RexUUID id;

        /// Asset ID.
        RexUUID asset_id;

        /// Asset type.
        asset_type_t asset_type;

        /// Inventory type.
        inventory_type_t inventory_type;

        /// Name.
        std::string name;

        /// Description.
        std::string description;
    };

    class OSPROTO_MODULE_API InventoryFolderSkeleton
    {
    public:
        /// Default constructor.
        InventoryFolderSkeleton();

        /// Constructor overload.
        /// @param id ID.
        /// @param name Name.
        InventoryFolderSkeleton(const RexUUID &newid, const std::string &newname = "New Folder");

        /// Destructor.
        virtual ~InventoryFolderSkeleton() {}

        /// Adds child folder.
        InventoryFolderSkeleton *AddChildFolder(const InventoryFolderSkeleton &folder);

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolderSkeleton *GetFirstChildFolderByName(const char *searchName);

        /// @return Folder by the requested id or null if the folder isn't found.
        InventoryFolderSkeleton *GetChildFolderByID(const RexUUID &searchId);

        /// @return Does this folder have children.
        bool HasChildren() const { return children.size() != 0; }

        /// Prints out the whole inventory folder tree to stdout.
        void DebugDumpInventoryFolderStructure(int indentationLevel);

        typedef std::list<InventoryFolderSkeleton> FolderList;
        typedef std::list<InventoryFolderSkeleton>::iterator FolderIter;

        /// The UUID of this folder.
        RexUUID id;

        /// The human-readable name for this folder.
        std::string name;

        /// Type of the inventory.
        inventory_type_t type;

        /// Is this folder editable.
        bool editable;

        /// Parent folder. If this node is the root, this is null.
        InventoryFolderSkeleton *parent;

        /// List of children.
        FolderList children;

        /// ?
        int version;

        /// ?
        int type_default;
    };

    /// Inventory represents the hierarchy of an OpenSim inventory.
    class OSPROTO_MODULE_API InventorySkeleton
    {
    public:
        /// Default constructor.
        InventorySkeleton();

        /// Destructor.
        virtual ~InventorySkeleton() {}

        /// @return Inventory root folder.
        InventoryFolderSkeleton *GetRoot() { return &root_; }

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolderSkeleton *GetFirstChildFolderByName(const char *searchName);

        /// @return Folder by the requested id or null if the folder isn't found.
        InventoryFolderSkeleton *GetChildFolderByID(const RexUUID &searchId);

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolderSkeleton *GetMyInventoryFolder();

        /// Prints out the whole inventory folder tree to stdout.
        void DebugDumpInventoryFolderStructure();

        private:
        /// Root folder.
        InventoryFolderSkeleton root_;
    };
}

#endif
