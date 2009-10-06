// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file AbstractInventoryItem.h
 *  @brief 
 */

#ifndef incl_InventoryModule_AbstractInventoryItem_h
#define incl_InventoryModule_AbstractInventoryItem_h

#include <QObject>
#include <QString>

namespace Inventory
{
    class AbstractInventoryItem : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString id_ READ GetID WRITE SetID)
        Q_PROPERTY(QString name_ READ GetName WRITE SetName)
        Q_PROPERTY(AbstractInventoryItem *parent_ READ GetParent WRITE SetParent)
        Q_ENUMS(InventoryDataModel)
        Q_ENUMS(InventoryItemType)

    public:
        /// Enumeration of inventory data models.
        /// Add your own enum here if you plan making your own inventory data model.
        ///\todo Is this really needed?
        enum InventoryDataModel
        {
            IDM_OpenSim = 0,
            IDM_WebDav,
            IDM_Unknown
        };

        /// Enumeration of inventory item types.
        enum InventoryItemType
        {
            Type_Folder = 0,
            Type_Asset,
            Type_Unknown
        };

        /// Constructor.
        /// @param data_model Data model type.
        /// @param item_type Inventory item type.
        AbstractInventoryItem();

        /// Constructor.
        /// @param type Type (folder or item)
        /// @param id ID.
        /// @param type name Name.
        /// @param type parent Parent folder.
        AbstractInventoryItem(
            const InventoryItemType &item_type,
            const QString &id,
            const QString &name,
            AbstractInventoryItem *parent = 0) :
            itemType_(item_type),
            id_(id),
            name_(name),
            parent_(parent) {}

        /// Destructor.
        virtual ~AbstractInventoryItem() {}

        /// Set/get for name.
        QString GetName() const {return name_; }
        void SetName(const QString &name) { name_ = name; }

        /// Set/get for ID.
        QString GetID() const { return id_; }
        void SetID(const QString &id) { id_ = id; }

        /// Set/get for parent.
        AbstractInventoryItem *GetParent() const { return parent_; }
        void SetParent(AbstractInventoryItem *parent) { parent_ = parent; }

        /// @return Type of the item (folder/asset).
        InventoryItemType GetItemType() const { return itemType_; }

    private:
        Q_DISABLE_COPY(AbstractInventoryItem);

        /// Unique ID for this item.
        QString id_;

        /// Name.
        QString name_;

        /// Type of data model this item uses.
        ///\todo Is this needed?
        InventoryDataModel dataModel_;

        /// Type of item (folder or asset)
        InventoryItemType itemType_;

        /// Parent pointer.
        AbstractInventoryItem *parent_;
    };
}

#endif
