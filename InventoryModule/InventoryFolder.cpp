// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryFolder.cpp
 *  @brief A class representing inventory folder.
 */

#include "StableHeaders.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"
#include "RexUUID.h"

namespace Inventory
{

InventoryFolder::InventoryFolder() :
    AbstractInventoryItem(Type_Folder, QString(RexTypes::RexUUID().ToString().c_str()), "New Folder", 0),
    editable_(true), dirty_(false)
{
}

InventoryFolder::InventoryFolder(
    const QString &id,
    const QString &name,
    const bool &editable,
    InventoryFolder *parent) :
    AbstractInventoryItem(Type_Folder, id, name, parent),
    dirty_(false)
{
}

// virtual
InventoryFolder::~InventoryFolder()
{
    qDeleteAll(children_);
}

AbstractInventoryItem *InventoryFolder::AddChild(AbstractInventoryItem *child)
{
    child->SetParent(this);
    children_.append(child);
    return children_.back();
}

bool InventoryFolder::RemoveChildren(int position, int count)
{
    if (position < 0 || position + count > children_.size())
        return false;

    for(int row = 0; row < count; ++row)
        delete children_.takeAt(position);

    return true;
}

/*
void InventoryFolder::DeleteChild(InventoryItemBase *child)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if (item == child)
            SAFE_DELETE(item);
    }
}

void InventoryFolder::DeleteChild(const RexUUID &id)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if (item->GetID() == id)
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
*/

InventoryFolder *InventoryFolder::GetFirstChildFolderByName(const QString &searchName)
{
    if (GetName() == searchName)
        return this;

    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetItemType() == Type_Folder)
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

InventoryFolder *InventoryFolder::GetChildFolderByID(const QString &searchId)
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetItemType() == Type_Folder)
            folder = static_cast<InventoryFolder *>(item);
        else
            continue;

        if (folder->GetID() == searchId)
            return folder;

        InventoryFolder *folder2 = folder->GetChildFolderByID(searchId);
        if (folder2)
            if (folder2->GetID() == searchId)
                return folder2;
    }

    return 0;
}

InventoryAsset *InventoryFolder::GetChildAssetByID(const QString &searchId)
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryAsset *asset = 0;
        if (item->GetItemType() == Type_Asset)
            asset = static_cast<InventoryAsset *>(item);
        else
            continue;

        if (asset->GetID() == searchId)
            return asset;

    ///\todo Recursion, if needed.
/*
        InventoryFolder *folder = folder->GetChildFolderByID(searchId);
        if (folder)
            if (asset->GetID() == searchId)
                return folder2;
*/
    }

    return 0;
}

AbstractInventoryItem *InventoryFolder::Child(int row)
{
    return children_.value(row);
}

int InventoryFolder::ChildCount() const
{
    return children_.count();
}

void InventoryFolder::DebugDumpInventoryFolderStructure(int indentationLevel)
{
    for(int i = 0; i < indentationLevel; ++i)
        std::cout << " ";
    std::cout << GetName().toStdString() << " " << std::endl;

    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(it.next());
        if (folder)
            folder->DebugDumpInventoryFolderStructure(indentationLevel + 3);
    }
}

int InventoryFolder::Row() const
{
    if (GetParent())
    {
        InventoryFolder *folder = static_cast<InventoryFolder *>(GetParent());
        return folder->children_.indexOf(const_cast<InventoryFolder *>(this));
    }

    return 0;
}

}
