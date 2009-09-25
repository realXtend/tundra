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
        InventoryFolder(const RexUUID &id, const std::string &name = "New Folder", InventoryFolder *parent = 0);

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

        /// Returns pointer to requested folder, or null if not found.
        /// @param id Search ID.
        /// @return Pointer to requested folder, or null if not found.
        InventoryFolder *GetChildFolderByID(const RexUUID &id);

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

        /// @param column
        QVariant Data(int column) const;

        /// @return Row number of the folder.
        int Row() const;

        /// @return Can user edit this folder's name, delete or remove it.
        const bool IsEditable() const { return editable_; }

        ///\todo Find out is this needed?
        int version;

        /// Find out is this needed?
        int type_default;

    private:
        /// List of child items.
        QList<InventoryItemBase *> childItems_;

        ///
        QList<QVariant> itemData_;

        /// Is this folder editable.
        bool editable_;
    };
}

#endif