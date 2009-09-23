// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OSProto_Inventory_h
#define incl_OSProto_Inventory_h

#include "OpenSimProtocolModuleApi.h"
#include "RexUUID.h"

using namespace RexTypes;

class OSPROTO_MODULE_API InventoryItem
{
public:
    /// Constructor.
    /// @param inventory_id Inventory ID.
    /// @param asset_id Asset ID.
    /// @param asset_type Asset type.
    InventoryItem(const RexUUID &inventory_id, const RexUUID &asset_id, asset_type_t asset_type);

    /// Default destructor.
    virtual ~InventoryItem() {}

    /// @return Inventory ID.
    const RexUUID &GetInventoryID() const { return inventoryID_;}

    /// @return Asset ID.
    const RexUUID &GetAssetID() const { return assetID_; }

    /// @return Asset type.
    const asset_type_t GetAssetType() const { return assetType_; }

    /// @return Inventory type.
    const inventory_type_t GetInventoryType() const { return inventoryType_; }

    /// Sets the name.
    /// @param name New name.
    void SetName(const std::string &name) { name_ = name; }

    /// @return Name.
    const std::string &GetName() const { return name_;}

    /// Sets the description.
    /// @param description New description.
    void SetDescription(const std::string &description) { description_ = description; }

    /// @return Description.
    const std::string &GetDescription() const { return description_;}

private:
    /// Inventory item ID.
    RexUUID inventoryID_;

    /// Asset ID.
    RexUUID assetID_;

    /// Asset type.
    asset_type_t assetType_;

    /// Inventory type.
    inventory_type_t inventoryType_;

    /// Name.
    std::string name_;

    /// Description.
    std::string description_;
};

class OSPROTO_MODULE_API InventoryFolder
{
public:
    InventoryFolder()
    :parent_(0), version(0), type_default(0)
    {
    }

    ///\todo This function should not exist. Set the id from the ctor.
    void SetID(const RexUUID &id) { id_ = id; }

    /// @return Inventory folder ID.
    const RexUUID &GetID() const { return id_; }

    /// @return Inventory folder ID.
    RexUUID GetParentID() const;

    /// Sets the name of the folder.
    /// @param name New name.
    void SetName(const std::string &name) { name_ = name; }

    /// @return Name of the folder.
    std::string GetName() const { return name_; }

    /// Sets the type of the folder.
    void SetType(const inventory_type_t &type){ type_ = type; }

    /// @return Type of the folder.
    const inventory_type_t &GetType() const { return type_; }

    /// @return First folder by the requested name or null if the folder isn't found.
    InventoryFolder *GetFirstSubFolderByName(const char *name);

    /// @return First folder by the requested id or null if the folder isn't found.
    InventoryFolder *GetFirstSubFolderByID(const RexUUID &id);

    /// @return List of child folder in this folder.
    std::list<InventoryFolder> &GetChildren() { return children_; }

    /// @return List of items in this folder.
    std::list<InventoryItem> &GetItems() { return items_; }

    /// Creates a new subfolder into this folder.
    /// Note: Copies the passed InventoryFolder structure to the folder tree - so the object that was passed
    /// as param doesn't become a part of this folder tree.
    /// @return The newly added folder.
    InventoryFolder *AddSubFolder(const InventoryFolder &folder);

    /// Adds new item to the folder.
    /// @param item New item.
    void AddItem(const InventoryItem &item);

    /// Returns inventory item or null if not found.
    /// @param id Inventory ID.
    /// @return Inventory item.
    InventoryItem *GetItemByInventoryID(const RexUUID &inventory_id);

    /// Returns inventory item or null if not found.
    /// @param id Asset ID.
    /// @return Inventory item.
    /// @note Multiple inventory items can share the same asset ID. Returns the fist occurence.
    InventoryItem *GetFirstItemByAssetID(const RexUUID &asset_id);

    /// Returns inventory item or null if not found.
    /// @param asset_id Asset ID.
    /// @param inventory_id Inventory ID.
    /// @return Inventory item.
    InventoryItem *GetItemByBothIDs(const RexUUID &asset_id, const RexUUID &inventory_id);

    /// ?
    int version;
    /// ?
    int type_default;

#ifdef _DEBUG
    void DebugDumpInventoryFolderStructure(int indentationLevel);
#endif

private:
    /// The UUID of this folder.
    RexUUID id_;

    /// The human-readable name for this folder.
    std::string name_;

    /// Type of the inventory.
    inventory_type_t type_;

    /// Parent folder. If this node is the root, this is null.
    InventoryFolder *parent_;

    typedef std::list<InventoryFolder> FolderList;
    typedef std::list<InventoryFolder>::iterator FolderIter;

    /// List of child folders.
    FolderList children_;

    typedef std::list<InventoryItem> ItemList;
    typedef std::list<InventoryItem>::iterator ItemIter;

    /// List of inventory items in this folder.
    ItemList items_;
};

/// Inventory represents the hierarchy of an OpenSim inventory.
class OSPROTO_MODULE_API Inventory
{
public:
    ///\todo This shoul not be public. Set the root from the inventory ctor.
    void SetRoot(InventoryFolder &r) { root = r; }

    /// @return Inventory root folder.
    InventoryFolder *GetRoot() { return &root; }

    /// @return First subfolder by the requested name, or null if not found.
    InventoryFolder *GetFirstSubFolderByName(const char *name);

    /// @return First subfolder by the requested id, or null if not found.
    InventoryFolder *GetFirstSubFolderByID(const RexUUID &id);

    /// @param id The id for the new folder, must be unique.
    /// @param parent The folder under which this new folder will be created.
    /// @return An existing folder with the given id, or if it doesn't exist, creates one under the parent folder and returns it.
    ///     If a new folder was created, the name will be "New Folder", so be sure to change it.
    InventoryFolder *GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent);

#ifdef _DEBUG
    /// Prints out the whole inventory folder tree to stdout.
    void DebugDumpInventoryFolderStructure();
#endif

private:
    /// The root folder.
    InventoryFolder root;
};

#endif
