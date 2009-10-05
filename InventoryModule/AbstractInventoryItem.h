// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file AbstractInventoryItem.h
 *  @brief 
 */

#ifndef incl_InventoryModule_AbstractInventoryItem_h
#define incl_InventoryModule_AbstractInventoryItem_h

#include <QObject>

namespace Inventory
{
    class AbstractInventoryItem;

    /// Enumeration of inventory data models.
    enum InventoryDataModel
    {
        IDM_OpenSim = 0,
        IDM_WebDav,
        IDM_Unknown
    };

    class AbstractInventoryItem : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param data_model
        AbstractInventoryItem(const InventoryDataModel &data_model);

        /// Destructor.
        virtual ~AbstractInventoryItem();

    private:
        Q_DISABLE_COPY(AbstractInventoryItem);

        /// Unique ID for this item.
        QString id_;

        /// Asset this item reference to.
        QString assetReference_;

        /// Type of data model this item uses.
        InventoryDataModel dataModel_;

        /// Parent pointer.
        AbstractInventoryItem *parent;
    };
}

#endif
