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
        InventoryFolder(const RexTypes::RexUUID &id, const std::string &name = "New Folder", InventoryFolder *parent = 0);

        /// Destructor.
        virtual ~InventoryFolder();

        /// Copy constructor.
        InventoryFolder(const InventoryFolder &rhs);

        /// Assign operator.
        InventoryFolder &operator = (const InventoryFolder &other);

        /// Adds new child.
        /// @param child
        InventoryItemBase *AddChild(InventoryItemBase *child);

        /// Adds new child.
        /// @param child
//        InventoryFolder *AddChildFolder(const InventoryFolder &child);

        /// Adds new child.
        /// @param child
//        InventoryAsset *AddChildAsset(const InventoryAsset &child);

        /// @return True if child of this parent.
        /// @param child Child folder.
        const bool IsChild(InventoryFolder *child);

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolder *GetFirstChildFolderByName(const char *name);

        /// @return
        /// @param id
        InventoryFolder *GetChildFolderByID(const RexTypes::RexUUID &id);

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

        /// Deprecated, delete?
        int version;

        /// Deprecated, delete?
        int type_default;

    private:
        /// List of child items.
        QList<InventoryItemBase *> childItems_;

        ///
        QList<QVariant> itemData_;
    };
}

#endif