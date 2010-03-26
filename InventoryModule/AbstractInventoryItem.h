// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AbstractInventoryItem.h
 *  @brief  Abstract inventory item. Subclass your inventory items from this class.
 *          If you create new type of item, remeber to add it to InventoryItemType enum.
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
        Q_PROPERTY(bool editable_ READ IsEditable WRITE SetEditable)
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

        /// @return Name.
        virtual QString GetName() const = 0;

        /// @param name New name.
        virtual void SetName(const QString &name) = 0;

        /// @return ID.
        virtual QString GetID() const = 0;

        /// @param id New ID.
        virtual void SetID(const QString &id) = 0;

        /// @return Parent pointer.
        virtual AbstractInventoryItem *GetParent() const = 0;

        /// @param parent New parent pointer.
        virtual void SetParent(AbstractInventoryItem *parent) = 0;

        /// @return Is this item editable.
        virtual bool IsEditable() const = 0;

        /// @param editable Is this item editable.
        virtual void SetEditable(const bool &editable) = 0;

        /// @return Is this item library asset.
        virtual bool IsLibraryItem() const = 0;

        /// @param value Is this item library asset.
        virtual void SetIsLibraryItem(const bool &value) = 0;

        /// @return Type of the item.
        virtual InventoryItemType GetItemType() const = 0;

        /// Is this folder descendent of spesific folder.
        /// @param searchFolder Folder to be investigated.
        virtual bool IsDescendentOf(AbstractInventoryItem *searchFolder) = 0;

    protected:
        /// Unique ID for this item.
        QString id_;

        /// Name.
        QString name_;

        /// Parent pointer.
        AbstractInventoryItem *parent_;

        /// Editable flag.
        bool editable_;

    private:
        Q_DISABLE_COPY(AbstractInventoryItem);
    };
}

#endif
