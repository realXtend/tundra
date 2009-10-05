// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.h
 *  @brief 
 */

#ifndef incl_InventoryModule_InventoryAsset_h
#define incl_InventoryModule_InventoryAsset_h

#include "AbstractInventoryItem.h"

namespace Inventory
{
    class InventoryAsset : public AbstractInventoryItem
    {
        Q_OBJECT

    public:
        /// Default constructor.
        /// @param model
        InventoryAsset(const InventoryDataModel &data_model);

        /// Destructor.
        virtual ~InventoryAsset();

    private:
        Q_DISABLE_COPY(InventoryAsset);
    };
}

#endif
