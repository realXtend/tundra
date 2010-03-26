// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.cpp
 *  @brief A class representing asset in inventory.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InventoryAsset.h"
#include "InventoryFolder.h"
#include "MemoryLeakCheck.h"

namespace Inventory
{

InventoryAsset::InventoryAsset(
    const QString &id,
    const QString &asset_reference,
    const QString &name,
    InventoryFolder *parent) :
    AbstractInventoryItem(id, name, parent),
    itemType_(AbstractInventoryItem::Type_Asset),
    assetReference_(asset_reference),
    creatorId_(RexUUID()),
    ownerId_(RexUUID()),
    groupId_(RexUUID()),
    creationTime_(time(0)),
    libraryItem_(false)
{
}

// virtual
InventoryAsset::~InventoryAsset()
{
}

bool InventoryAsset::IsDescendentOf(AbstractInventoryItem *searchFolder)
{
    forever
    {
        AbstractInventoryItem *parent = GetParent();
        if (parent)
            if (parent == searchFolder)
                return true;
            else
                return parent->IsDescendentOf(searchFolder);

        return false;
    }
}

/*
int InventoryAsset::Row() const
{
    if (GetParent())
        return GetParent()->Children().indexOf(const_cast<InventoryAsset *>(this));

    return 0;
}
*/

QString InventoryAsset::GetCreationTimeString() const
{
    QString str = asctime(localtime(&creationTime_));
    // remove \n from the end
    str.remove(QChar('\n'), Qt::CaseInsensitive);
    return str;
}
}
