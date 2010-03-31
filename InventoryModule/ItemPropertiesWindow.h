// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   ItemPropertiesWindow.h
 *  @brief  Item properties window. Shows basic information about inventory item and the asset
 *          it's referencing to.
 */

#ifndef incl_InventoryModule_ItemPropertiesWindow_h
#define incl_InventoryModule_ItemPropertiesWindow_h

#include <RexUUID.h>

#include <boost/shared_ptr.hpp>

#include <QWidget>
#include <QMap>

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace UiServices
{
    class UiProxyWidget;
}

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QLineEdit;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

namespace Inventory
{
    class InventoryModule;
    class InventoryAsset;

    class ItemPropertiesWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param owner InventoryModule pointer.
        /// @param parent Parent widget.
        ItemPropertiesWindow(InventoryModule *owner, QWidget *parent = 0);

        /// Destructor.
        ~ItemPropertiesWindow();

    public slots:
        /// Sets the item which properties we want to view.
        /// @param item Inventory asset.
        void SetItem(InventoryAsset *item);

        /// Set the file size shown in the UI.
        /// @param file_size File size.
        void SetFileSize(size_t file_size);

        /// Handles UuidNameReply 
        /// Inserts the corresponding name to the UI if it matches with the UUID.
        /// @param uuid_name_map Map of UUID-name pairs.
        void HandleUuidNameReply(QMap<RexUUID, QString> uuid_name_map);

    signals:
        /// Emitted when the window is closed.
        /// @param inventory_id Id of the inventory item.
        /// @param save_changes Do we want to save changes.
        void Closed(const QString &inventory_id, bool save_changes);

    private slots:
        /// Tells InventoryModule to destroy the window and save the changes made to the item.
        void Save();

        /// Tells InventoryModule to destroy the window. Doesn't save changes made to the item.
        void Cancel();

        /// Checks the validity of user-editable fiels.
        /// Disables Save button if no modifications are made.
        /// @return True if values were modified, false otherwise.
        bool EditingFinished();

    private:
        /// Main widget loaded from .ui file.
        QWidget *mainWidget_;

        /// Layout 
        QVBoxLayout *layout_;

        /// InventoryModule pointer.
        InventoryModule *owner_;

        /// Proxy widget for ui
        UiServices::UiProxyWidget *proxyWidget_;

        /// Inventory item ID
        QString inventoryId_;

        /// Original name of the item.
        QString originalName_;

        /// Original description of the item.
        QString originalDescription_;

        /// Creator ID.
        RexUUID creatorId_;

        /// Owner ID.
        RexUUID ownerId_;

        /// Group ID.
        RexUUID groupId_;

        /// Line edit for name.
        QLineEdit *lineEditName_;

        /// Line edit for description.
        QLineEdit *lineEditDescription_;

        /// Asset ID value label.
        QLabel *labelAssetIdData_;

        /// Type value label.
        QLabel *labelTypeData_;

        /// File size value label
        QLabel *labelFileSizeData_;

        /// Creation time value label.
        QLabel *labelCreationTimeData_;

        /// Creator value label.
        QLabel *labelCreatorData_;

        /// Owner value label.
        QLabel *labelOwnerData_;

        /// Group value label.
        QLabel *labelGroupData_;

        /// Save button.
        QPushButton *pushButtonSave_;

        /// Cancel buytton.
        QPushButton *pushButtonCancel_;
    };
}

#endif
