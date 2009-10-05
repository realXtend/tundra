// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryFolder.h
 *  @brief A class representing inventory folder.
 */

#ifndef InventoryFolder_h
#define InventoryFolder_h

#include "OpenSimProtocolModuleApi.h"
#include "InventoryItemBase.h"
#include "InventoryAsset.h"

#include <QList>
#include <QVariant>

namespace OpenSimProtocol
{
    class OSPROTO_MODULE_API InventoryFolder : public InventoryItemBase
    {
    public:
        /// Default constructor.
        InventoryFolder();

        /// Constructor.
        /// @param id ID.
        /// @param name Name.
        /// @param parent Parent folder.
        InventoryFolder(
            const RexUUID &id,
            const std::string &name = "New Folder",
            const bool &editable = true,
            InventoryFolder *parent = 0);

        /// Destructor.
        virtual ~InventoryFolder();

        /// Copy constructor.
        InventoryFolder(const InventoryFolder &rhs);

        /// Assign operator.
        InventoryFolder &operator = (const InventoryFolder &other);

        /// Adds new child.
        /// @param child Child to be added.
        /// @return Pointer to the new child.
        InventoryItemBase *AddChild(InventoryItemBase *child);

        /// Deletes child.
        /// @param position 
        /// @param count 
        /// @return True if removing is succesfull, false otherwise.
        /// @note It's not recommended to use this directly. This function is used by InventoryModel::removeRows().
        bool RemoveChildren(int position, int count);

        /// Deletes child.
        /// @param child Child to be deleted.
        void DeleteChild(InventoryItemBase *child);

        /// Deletes child.
        /// @param id of the child to be deleted.
        void DeleteChild(const RexUUID &id);

        /// @return True if child of this parent.
        /// @param child Child folder.
        const bool IsChild(InventoryFolder *child);

        /// @return First folder by the requested name or null if the folder isn't found.
        /// @param name Search name.
        /// @return Pointer to requested folder, or null if not found.
        InventoryFolder *GetFirstChildFolderByName(const char *name);

        /// Returns pointer to requested folder.
        /// @param searchId Search ID.
        /// @return Pointer to the requested folder, or null if not found.
        InventoryFolder *GetChildFolderByID(const RexUUID &searchId);

        /// Returns pointer to requested asset.
        /// @param searchId Search ID.
        /// @return Pointer to the requested asset, or null if not found.
        InventoryAsset *GetChildAssetByID(const RexUUID &searchId);

        /// Sets the folder dirty.
        void SetDirty(const bool &dirty) { dirty_ = dirty; }

        /// Is this folder dirty (the descendents haven't been fetched yet or they have been modified).
        const bool &IsDirty() const { return dirty_; }

        /// @param row
        /// @return
        InventoryItemBase *Child(int row);

        /// @return List of children.
        QList<InventoryItemBase *> &Children();

        /// @return Number of children.
        int ChildCount() const;

        /// @return Numbve
        ///\todo Probably not needed. Delete?
        int ColumnCount() const;

        ///
        /// @param column
        /// @param value
        /// @return True if succesfull, false otherwise.
        bool SetData(int column, const QVariant &value);

        /// @param column
        QVariant Data(int column) const;

        /// @return Row number of the folder.
        int Row() const;

        ///\todo Find out is this needed?
        int version;

        /// Find out is this needed?
        int type_default;

    private:
        /// List of child items.
        QList<InventoryItemBase *> childItems_;

        ///
        QList<QVariant> itemData_;

        /// Is this folder dirty.
        bool dirty_;
    };
}

#endif