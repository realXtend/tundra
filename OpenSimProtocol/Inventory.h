// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OSProto_Inventory_h
#define incl_OSProto_Inventory_h

#include "OpenSimProtocolModuleApi.h"
#include "RexUUID.h"

class OSPROTO_MODULE_API InventoryFolder
{
public:
    InventoryFolder()
    :parent(0), version(0), type_default(0)
    {
    }

    /// The UUID of this folder.
    RexTypes::RexUUID id;

    /// The human-readable name for this folder.
    std::string name;

    /// If this node is the root, this is null.
    InventoryFolder *parent;

    int version;
    int type_default;

    typedef std::list<InventoryFolder> FolderList;
    FolderList children;

    InventoryFolder *GetFirstSubFolderByName(const char *name);

    InventoryFolder *GetFirstSubFolderByID(const RexTypes::RexUUID &id);

    /// Creates a new subfolder into this folder.
    /// Note: Copies the passed InventoryFolder structure to the folder tree - so the object that was passed
    /// as param doesn't become a part of this folder tree.
    /// @return The newly added folder.
    InventoryFolder *AddSubFolder(const InventoryFolder &folder);

#ifdef _DEBUG
    void DebugDumpInventoryFolderStructure(int indentationLevel);
#endif
};

/// Inventory represents the hierarchy of an OpenSim inventory.
class OSPROTO_MODULE_API Inventory
{
public:
    InventoryFolder root;

    InventoryFolder *GetFirstSubFolderByName(const char *name);

    InventoryFolder *GetFirstSubFolderByID(const RexTypes::RexUUID &id);

    /// Prints out the whole inventory folder tree to stdout.
    void DebugDumpInventoryFolderStructure();

    /// @param id The id for the new folder, must be unique.
    /// @param parent The folder under which this new folder will be created.
    /// @return An existing folder with the given id, or if it doesn't exist, creates one under the parent folder and returns it.
    ///     If a new folder was created, the name will be "New Folder", so be sure to change it.
    InventoryFolder *GetOrCreateNewFolder(const RexTypes::RexUUID &id, InventoryFolder &parent);
};

#endif
