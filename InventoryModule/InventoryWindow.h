// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryWindow.h
 *  @brief  Inventory window. Should be totally unaware of the underlaying inventory data model.
 */

#ifndef incl_InventoryModule_InventoryWindow_h
#define incl_InventoryModule_InventoryWindow_h

#include "RexTypes.h"

#include <boost/shared_ptr.hpp>

#include <QPointer>
#include <QObject>
#include <QMap>

QT_BEGIN_NAMESPACE
class QPushButton;
class QTreeWidgetItem;
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Foundation
{
    class Framework;
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace UiServices
{
    class UiProxyWidget;
}

namespace Inventory
{
    class InventoryTreeView;
    class InventoryItemEventData;
    class InventoryItemModel;
    class AbstractInventoryDataModel;

    typedef boost::shared_ptr<AbstractInventoryDataModel> InventoryPtr;

    class InventoryWindow : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        InventoryWindow(Foundation::Framework *framework);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        /// Initializes the inventory data/view model.
        /// @param inventory_model Inventory data model pointer.
        void InitInventoryTreeModel(InventoryPtr inventory_model);

        /// Resets the inventory tree model.
        void ResetInventoryTreeModel();

        /// Hides the inventory window.
        void Hide();

    private slots:
        /// Opens inventory item (folder or asset) when user double-clicks it.
        void OpenItem();

        /// Adds new folder.
        void AddFolder();

        /// Deletes item (folder/asset).
        void DeleteItem();

        /// Renames item (folder/asset).
        void RenameItem();

        /// Opens file open dialog.
        void Upload();

        /// Uploads files.
        /// @param filenames list of filenames to upload.
        void UploadFiles(const QStringList &filenames);

        /// File download.
        void Download();

        /// Updates possible actions depending on the currently active tree view item.
        void UpdateActions();

        /// Opens download progress dialog.
        /// @param asset_id Asset id.
        void OpenDownloadProgess(const QString &asset_id);

        /// Aborts download.
        /// @param asset_id Asset id.
        void AbortDownload(const QString &asset_id);

        /// Closes download progress dialog.
        /// @param asset_id Asset id.
        void CloseDownloadProgess(const QString &asset_id);

    private:
        Q_DISABLE_COPY(InventoryWindow);

        /// Initializes the inventory UI.
        void InitInventoryWindow();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Inventory view model.
        QPointer<InventoryItemModel> inventoryItemModel_;

        /// Inventory window widget.
        QWidget *inventoryWidget_;

        // Inventory tree view widget
        InventoryTreeView *treeView_;

        /// Close button.
        QPushButton *buttonClose_;

        /// Download button.
        QPushButton *buttonDownload_;

        /// Upload button.
        QPushButton *buttonUpload_;

        /// Add folder button.
        QPushButton *buttonAddFolder_;

        /// Delete item button.
        QPushButton *buttonDeleteItem_;

        /// Rename button.
        QPushButton *buttonRename_;

        /// Proxy Widget for ui
        UiServices::UiProxyWidget *proxyWidget_;
    };
}

#endif
