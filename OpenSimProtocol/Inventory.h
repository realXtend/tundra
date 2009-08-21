// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OSProto_Inventory_h
#define incl_OSProto_Inventory_h

#include "RexUUID.h"

class InventoryFolder;

class InventoryFolder
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

    void AddSubFolder(const InventoryFolder &folder);

    void DebugDumpInventoryFolderStructure(int indentationLevel);
};

/// Inventory represents the hierarchy of an OpenSim inventory.
class Inventory
{
public:
    InventoryFolder root;

    InventoryFolder *GetFirstSubFolderByName(const char *name);

    InventoryFolder *GetFirstSubFolderByID(const RexTypes::RexUUID &id);

    /// Prints out the whole inventory folder tree to stdout.
    void DebugDumpInventoryFolderStructure();
};

#endif
