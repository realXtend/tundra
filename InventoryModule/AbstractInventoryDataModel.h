// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file AbstractInventoryDataModel.h
 *  @brief 
 */

#ifndef incl_InventoryModule_AbstractInventoryDataModel_h
#define incl_InventoryModule_AbstractInventoryDataModel_h

#include "AbstractInventoryItem.h"

#include <QObject>

namespace Inventory
{
    class AbstractInventoryDataModel : public QObject
    {
        Q_OBJECT

    public:
        /// Default constructor.
        AbstractInventoryDataModel();

        /// Destructor.
        virtual ~AbstractInventoryDataModel();

        ///
        virtual void AddFolder(AbstractInventoryItem *newFolder, AbstractInventoryItem *parentFolder) = 0;

    private:
        Q_DISABLE_COPY(AbstractInventoryDataModel);

        AbstractInventoryItem rootFolder_;
    };
}
#endif
