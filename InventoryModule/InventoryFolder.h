// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryFolder.h
 *  @brief 
 */

#ifndef incl_InventoryModule_InventoryFolder_h
#define incl_InventoryModule_InventoryFolder_h

#include "AbstractInventoryItem.h"

namespace Inventory
{
    class InventoryAsset;

    class InventoryFolder : public AbstractInventoryItem
    {
        Q_OBJECT
        Q_PROPERTY(bool editable_ READ IsEditable WRITE SetEditable)
        Q_PROPERTY(bool dirty_ READ IsDirty WRITE SetDirty)

    public:
        /// Default constructor.
        InventoryFolder();

        /// Constructor.
        /// @param data_model Data model.
        /// @param id ID.
        /// @param name Name.
        /// @param parent Parent folder pointer.
        InventoryFolder(
            //const InventoryDataModel &data_model,
            const QString &id,
            const QString &name = "New Folder",
            const bool &editable = true,
            InventoryFolder *parent = 0);

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
        /// @note It's not recommended to use this directly. This function is used by InventoryViewModel::removeRows().
        bool RemoveChildren(int position, int count);

        /// Deletes child.
        /// @param child Child to be deleted.
        void DeleteChild(AbstractInventoryItem *child);

        /// Deletes child.
        /// @param id of the child to be deleted.
        void DeleteChild(const QString &id);

        /// @return First folder by the requested name or null if the folder isn't found.
        /// @param name Search name.
        /// @return Pointer to requested folder, or null if not found.
        InventoryFolder *GetFirstChildFolderByName(const QString &name);

        /// Returns pointer to requested folder.
        /// @param searchId Search ID.
        /// @return Pointer to the requested folder, or null if not found.
        InventoryFolder *GetChildFolderByID(const QString &searchId);

        /// Returns pointer to requested asset.
        /// @param searchId Search ID.
        /// @return Pointer to the requested asset, or null if not found.
        InventoryAsset *GetChildAssetByID(const QString &searchId);

        /// Get/set for the dirty flag.
        bool IsDirty() const { return dirty_; }
        void SetDirty(const bool &dirty) { dirty_ = dirty; }

        /// Get/set for the dirty flag.
        bool IsEditable() const { return editable_; }
        void SetEditable(const bool &editable) { editable_ = editable; }

        /// @param row
        /// @return
        AbstractInventoryItem *Child(int row);

        /// @return Number of children.
        int ChildCount() const;

        /// @return Numbve
        ///\todo Probably not needed. Delete?
        int ColumnCount() const;

        /// @return Row number of the folder.
        int Row() const;

    private:
        Q_DISABLE_COPY(InventoryFolder);

        /// List of children.
        QList<AbstractInventoryItem *> children_;

        /// Dirty flag.
        bool dirty_;

        /// Editable flag.
        bool editable_;
    };
}

#endif
