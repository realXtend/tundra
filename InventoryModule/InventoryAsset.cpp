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

InventoryAsset::InventoryAsset(const QString &id, const QString &asset_reference,const QString &name, InventoryFolder *parent) :
    AbstractInventoryItem(id, name, parent), itemType_(AbstractInventoryItem::Type_Asset), assetReference_(asset_reference),
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
        {
            if (parent == searchFolder)
                return true;
            else
                return parent->IsDescendentOf(searchFolder);
        }

        return false;
    }
}

/*
int InventoryAsset::ColumnCount() const
{
    return itemData_.count();
}

int InventoryAsset::Row() const
{
    if (GetParent())
        return GetParent()->Children().indexOf(const_cast<InventoryAsset *>(this));

    return 0;
}
*/

}
