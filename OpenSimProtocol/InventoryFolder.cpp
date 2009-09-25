// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryFolder.cpp
 *  @brief A class representing inventory folder.
 */

#include "StableHeaders.h"
#include "InventoryFolder.h"

namespace OpenSimProtocol
{

InventoryFolder::InventoryFolder(): InventoryItemBase(Type_Folder, RexUUID(), "New Folder", 0)
{
}

InventoryFolder::InventoryFolder(const RexUUID &id, const std::string &name, InventoryFolder *parent) :
    InventoryItemBase(Type_Folder, id, name, parent)
{
    itemData_ << name.c_str();
}

// virtual
InventoryFolder::~InventoryFolder()
{
    qDeleteAll(childItems_);
}

InventoryFolder::InventoryFolder(const InventoryFolder &rhs) : InventoryItemBase(rhs)
{
    itemData_ = rhs.itemData_;
    type_default = rhs.type_default;
    version = rhs.version;
    childItems_ = rhs.childItems_;
}

InventoryFolder &InventoryFolder::operator = (const InventoryFolder &rhs)
{
    InventoryItemBase::operator=(rhs);
    itemData_ = rhs.itemData_;
    type_default = rhs.type_default;
    version = rhs.version;
    childItems_ = rhs.childItems_;
    return *this;
}

InventoryItemBase *InventoryFolder::AddChild(InventoryItemBase *child)
{
    child->SetParent(this);
    childItems_.append(child);
    return childItems_.back();
}

void InventoryFolder::DeleteChild(InventoryItemBase *child)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if(item == child)
            SAFE_DELETE(item);
    }
}

void InventoryFolder::DeleteChild(const RexUUID &id)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if(item->GetID() == id)
            SAFE_DELETE(item);
    }
}

const bool InventoryFolder::IsChild(InventoryFolder *child)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
        if (folder)
        {
            if (folder == child)
                return true;
            
                if (folder->IsChild(child))
                    return true;
        }
    }

    return false;
}

InventoryFolder *InventoryFolder::GetFirstChildFolderByName(const char *searchName)
{
    if (name_ == searchName)
        return this;

    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetInventoryItemType() == Type_Folder)
            folder = static_cast<InventoryFolder *>(item);
        else
            continue;

        if (folder->GetName() == searchName)
            return folder;

        InventoryFolder *folder2 = folder->GetFirstChildFolderByName(searchName);
        if (folder2)
            if (folder2->GetName() == searchName)
                return folder2;
    }

    return 0;
}

InventoryFolder *InventoryFolder::GetChildFolderByID(const RexUUID &id)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetInventoryItemType() == Type_Folder)
            folder = static_cast<InventoryFolder *>(item);
        else
            continue;

        if (folder->GetID() == id)
            return folder;

        InventoryFolder *folder2 = folder->GetChildFolderByID(id);
        if (folder2)
            if (folder2->GetID() == id)
                return folder2;
    }

    return 0;
}

InventoryItemBase *InventoryFolder::Child(int row)
{
    return childItems_.value(row);
}

QList<InventoryItemBase *> &InventoryFolder::Children()
{
    return childItems_;
}

int InventoryFolder::ChildCount() const
{
    return childItems_.count();
}

int InventoryFolder::ColumnCount() const
{
    return itemData_.count();
}

QVariant InventoryFolder::Data(int column) const
{
    return itemData_.value(column);
}

int InventoryFolder::Row() const
{
    if (GetParent())
        return GetParent()->childItems_.indexOf(const_cast<InventoryFolder *>(this));

    return 0;
}

}
