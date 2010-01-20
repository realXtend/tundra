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
class QMenu;
class QAction;
class QMessageBox;
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

        /// Copies asset reference (UUID or URL) to the clipboard.
        void CopyAssetReference();

        /// Updates possible actions depending on the currently active tree view item.
        void UpdateActions();

        /// Opens download progress dialog.
        /// @param asset_id Asset id.
        /// @param name Asset name.
        void OpenDownloadProgess(const QString &asset_id, const QString &name);

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

        /// Creates the context menu actions.
        void CreateActions();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Inventory view model.
        QPointer<InventoryItemModel> inventoryItemModel_;

        /// Inventory window widget.
        QWidget *inventoryWidget_;

        // Inventory tree view widget
        InventoryTreeView *treeView_;

        /// Proxy Widget for ui
        UiServices::UiProxyWidget *proxyWidget_;

        /// Action menu.
        QMenu *actionMenu_;

        /// Delete action.
        QAction *actionDelete_;

        /// Rename action.
        QAction *actionRename_;

        /// Cut action.
        QAction *actionCut_;

        /// Paste action.
        QAction *actionPaste_;

        /// New Folder action.
        QAction *actionNewFolder_;

        /// Open action.
        QAction *actionOpen_;

        /// Copy asset reference action.
        QAction *actionCopyAssetReference_;

        /// Upload action.
        QAction *actionUpload_;

        /// Download action.
        QAction *actionDownload_;

        /// Separator (action) for the context menu.
        QAction *actionSeparator_;

        /// Map of active download progress dialogs.
        QMap<QString, QMessageBox *> downloadDialogs_;

        /// Offset for download dialog positions.
        size_t offset_;
    };
}

#endif
