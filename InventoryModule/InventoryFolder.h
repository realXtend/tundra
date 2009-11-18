// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryFolder.h
 *  @brief A class representing folder in the inventory item tre model.
 */

#ifndef incl_InventoryModule_InventoryFolder_h
#define incl_InventoryModule_InventoryFolder_h

#include "StableHeaders.h"
#include "AbstractInventoryItem.h"

namespace Inventory
{
    class InventoryAsset;

    class InventoryFolder : public AbstractInventoryItem
    {
        Q_OBJECT
        Q_PROPERTY(bool dirty_ READ IsDirty WRITE SetDirty)

    public:
        /// Constructor.
        /// @param data_model Data model.
        /// @param id ID.
        /// @param name Name.
        /// @param parent Parent folder pointer.
        InventoryFolder(const QString &id, const QString &name = "New Folder", InventoryFolder *parent = 0,
            const bool &editable = true);

        /// Destructor.
        virtual ~InventoryFolder();

        /// Adds new child.
        /// @param child Child to be added.
        /// @return Pointer to the new child.
        AbstractInventoryItem *AddChild(AbstractInventoryItem *child);

        /// Deletes child.
        ///\todo Refactor. Don't use count.
        /// @param position 
        /// @param count 
        /// @return True if removing is succesfull, false otherwise.
        /// @note It's not recommended to use this directly. This function is used by InventoryItemModel::removeRows().
        bool RemoveChildren(int position, int count);

        /// Deletes child.
        /// @param child Child to be deleted.
//        void DeleteChild(AbstractInventoryItem *child);

        /// Deletes child.
        /// @param id of the child to be deleted.
//        void DeleteChild(const QString &id);

        /// @return First folder by the requested name or null if the folder isn't found.
        /// @param name Search name.
        /// @return Pointer to requested folder, or null if not found.
        InventoryFolder *GetFirstChildFolderByName(const QString &name);

        /// Returns pointer to requested folder.
        /// @param searchId Search ID.
        /// @return Pointer to the requested folder, or null if not found.
        InventoryFolder *GetChildFolderById(const QString &searchId);

        /// Returns pointer to requested asset.
        /// @param searchId Search ID.
        /// @return Pointer to the requested asset, or null if not found.
        InventoryAsset *GetChildAssetById(const QString &searchId);

        /// Returns pointer to requested child item.
        /// @param searchId Search ID.
        /// @return Pointer to the requested item, or null if not found.
        AbstractInventoryItem *GetChildById(const QString &searchId);

        /// Get/set for the dirty flag.
        bool IsDirty() const { return dirty_; }
        void SetDirty(const bool &dirty) { dirty_ = dirty; }

        /// Get/set for the libary asset flag.
        bool IsLibraryItem() const { return libraryAsset_; }
        void SetIsLibraryItem(const bool &value) { libraryAsset_ = value; }

        /// @return Type of the item.
        InventoryItemType GetItemType() const { return itemType_; }

        /// Is this folder descendent of spesific folder.
        /// @param searchFolder Folder to be investigated.
        bool IsDescendentOf(AbstractInventoryItem *searchFolder);

        /// @return List of all the id's of the descendents of this folder.
        QList<QString> GetDescendentIds();

        /// @param row Row number of wanted child.
        /// @return Child item.
        AbstractInventoryItem *Child(int row);

        /// @return Number of children.
        int ChildCount() const { return children_.count(); }

        /// @return Does this folder have children.
        bool HasChildren() { return ChildCount() > 0; }

        /// @return Row number of the folder.
        int Row() const;

        /// @return folders child list 
        QList<AbstractInventoryItem *> &GetChildList() { return children_; }

#ifdef _DEBUG
        /// Prints the inventory tree structure to std::cout.
        void DebugDumpInventoryFolderStructure(int indentationLevel);
#endif

    private:
        Q_DISABLE_COPY(InventoryFolder);

        /// Type of item (folder or asset)
        InventoryItemType itemType_;

        /// List of children.
        QList<AbstractInventoryItem *> children_;

        /// Dirty flag.
        bool dirty_;

        /// Library asset flag.
        bool libraryAsset_;
    };
}

#endif
