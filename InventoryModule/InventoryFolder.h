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
    class InventoryFolder : public AbstractInventoryItem
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param data_model
        InventoryFolder(const InventoryDataModel &data_model);

        /// Destructor.
        virtual ~InventoryFolder();

    private:
        Q_DISABLE_COPY(InventoryFolder);
    };
}

#endif
