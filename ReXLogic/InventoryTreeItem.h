// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeItem.h
 *  @brief A container for items of data supplied by the inventory tree model.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#ifndef InventoryTreeItem_h
#define InventoryTreeItem_h

#include "RexUUID.h"

#include <QList>
#include <QVariant>

enum InventoryItemType
{
    Type_Folder = 0,
    Type_Item
};

/************** AbstractInventoryTreeItem *****************/

class AbstractInventoryTreeItem
{
    friend class InventoryTreeFolder;

public:
    /// Constructor.
    /// @param type Type (folder or item)
    /// @param id ID.
    /// @param type name Name.
    AbstractInventoryTreeItem(
        const InventoryItemType &type,
        const RexTypes::RexUUID &id,
        const std::string &name,
        InventoryTreeFolder *parent = 0) :
        itemType_(type), id_(id), name_(name), parent_(parent) {}

    /// Destructor.
    virtual ~AbstractInventoryTreeItem() {}

    /// Sets the name.
    /// @param name Name.
    void SetName(const std::string name) { name_ = name; }

    /// @return Name.
    const std::string &GetName() const { return name_; }

    /// @return ID.
    const RexTypes::RexUUID &GetID() const { return id_; }

    /// @return Type.
    const InventoryItemType &GetInventoryItemType() const { return itemType_; }

    /// @return Parent.
    InventoryTreeFolder *Parent() const { return parent_; } 

protected:
    /// Type of the tree item.
    InventoryItemType itemType_;

    /// ID.
    RexTypes::RexUUID id_;

    /// Name.
    std::string name_;

    /// Parent.
    InventoryTreeFolder *parent_;
};

/************** InventoryTreeFolder *****************/

class InventoryTreeFolder : public AbstractInventoryTreeItem
{
public:
    /// Constructor.
    InventoryTreeFolder(
        const QList<QVariant> &data,
        const RexTypes::RexUUID &id,
        const std::string &name = "New Folder",
        InventoryTreeFolder *parent = 0);

    ///
    virtual ~InventoryTreeFolder();

    ///
    void AppendChild(AbstractInventoryTreeItem *child);

    ///
    AbstractInventoryTreeItem *Child(int row);

    ///
    QList<AbstractInventoryTreeItem *> Children();

    ///
    int ChildCount() const;

    ///
    int ColumnCount() const;

    ///
    QVariant Data(int column) const;

    ///
    int Row() const;

private:
    ///
    QList<AbstractInventoryTreeItem *> childItems_;

    ///
    QList<QVariant> itemData_;

};

/************** InventoryTreeItem *****************/

class InventoryTreeItem : public AbstractInventoryTreeItem
{
public:
    /// Constructor.
    /// @param data
    /// @param id ID.
    /// @param parent Parent folder.
    InventoryTreeItem(
        const QList<QVariant> &data,
        const RexTypes::RexUUID &id,
        const std::string &name = "New Item",
        InventoryTreeFolder *parent = 0);

    ///
    virtual ~InventoryTreeItem();

    ///
    int ColumnCount() const;

    ///
    QVariant Data(int column) const;

    ///
    int Row() const;

private:
    ///
    QList<QVariant> itemData_;

    /// ID of the asset this inventory item refers to.
    RexTypes::RexUUID assetID_;
};

#endif
