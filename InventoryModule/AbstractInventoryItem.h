// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file AbstractInventoryItem.h
 *  @brief 
 */

#ifndef incl_InventoryModule_AbstractInventoryItem_h
#define incl_InventoryModule_AbstractInventoryItem_h

#include <QObject>
#include <QString>
#include <QDataStream>

namespace Inventory
{
    class AbstractInventoryItem : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString id_ READ GetID WRITE SetID)
        Q_PROPERTY(QString name_ READ GetName WRITE SetName)
        Q_PROPERTY(AbstractInventoryItem *parent_ READ GetParent WRITE SetParent)
        Q_PROPERTY(bool editable_ READ IsEditable WRITE SetEditable)
        Q_ENUMS(InventoryDataModel)
        Q_ENUMS(InventoryItemType)

    public:
        /// Enumeration of inventory item types.
        enum InventoryItemType
        {
            Type_Folder = 0,
            Type_Asset,
            Type_Unknown
        };

        /// Constructor.
        /// @param type Type (folder or item)
        /// @param id ID.
        /// @param type name Name.
        /// @param type parent Parent folder.
        AbstractInventoryItem(const QString &id, const QString &name, AbstractInventoryItem *parent = 0,
            const bool &editable = true) : id_(id), name_(name), parent_(parent), editable_(editable) {}

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

        /// Get/set for the editable flag.
        bool IsEditable() const { return editable_; }
        void SetEditable(const bool &editable) { editable_ = editable; }

        /// @return Type of the item (folder/asset).
        virtual InventoryItemType GetItemType() const = 0;

        /// Get/set for library asset flag.
        virtual bool IsLibraryAsset() const = 0;
        virtual void SetIsLibraryAsset(const bool &value) = 0;

        /// Is this folder descendent of spesific folder.
        /// @param searchFolder Folder to be investigated.
        virtual bool IsDescendentOf(AbstractInventoryItem *searchFolder) = 0;

    private:
        Q_DISABLE_COPY(AbstractInventoryItem);

        /// Unique ID for this item.
        QString id_;

        /// Name.
        QString name_;

        /// Parent pointer.
        AbstractInventoryItem *parent_;

        /// Read-only flag.
        bool editable_;
    };
}

#endif
