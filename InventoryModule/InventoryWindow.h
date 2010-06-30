/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryWindow.h
 *  @brief  Inventory window. Should be totally unaware of the underlaying inventory data model.
 */

#ifndef incl_InventoryModule_InventoryWindow_h
#define incl_InventoryModule_InventoryWindow_h

#include <boost/shared_ptr.hpp>

#include <QWidget>
#include <QPointer>
#include <QMap>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QMenu;
class QAction;
class QMessageBox;
class QModelIndex;
class QLineEdit;
QT_END_NAMESPACE

namespace UiServices
{
    class ProgressController;
}

namespace CoreUi
{
    class NotificationBaseWidget;
}

namespace Inventory
{
    class InventoryTreeView;
    class InventoryItemModel;
    class AbstractInventoryDataModel;
    typedef boost::shared_ptr<AbstractInventoryDataModel> InventoryPtr;

    /// Inventory window
    class InventoryWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param parent Parent widget.
        explicit InventoryWindow(QWidget *parent = 0);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        /// Initializes the inventory data/view model.
        /// @param inventory_model Inventory data model pointer.
        void InitInventoryTreeModel(InventoryPtr inventory_model);

        /// Resets the inventory tree model.
        void ResetInventoryTreeModel();

    signals:
        /// Signal indicating that user has chosen to view item properties of inventory item.
        /// @param inventory_id Inventory ID of the item.
        void OpenItemProperties(const QString &inventory_id);

        /// Sends notification to the UI.
        /// @param widget Notification widget.
        void Notification(CoreUi::NotificationBaseWidget *widget);

    protected:
        /// QWidget override.
        void changeEvent(QEvent* e);

    private slots:
        /// Refreshes (collapses, opens and expands) item at spesific index. Used mostly for folders.
        /// @param index Model index.
        void IndexIsDirty(const QModelIndex &index);

        /// Opens inventory item (folder or asset) when user double-clicks it.
        void OpenItem();

        /// Expands the folder at spesific index.
        /// @param index Folder model index.
        void ExpandFolder(const QModelIndex &index);

        /// Opens inventory item's property window.
        void OpenItemProperties();

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
        void UploadFiles(QStringList &filenames);

        /// File download.
        void Download();

        /// Copies asset reference (UUID or URL) to the clipboard.
        void CopyAssetReference();

        /// Searchs inventory items by selected text. Currently searches only by the name of item.
        /// Sets the best matching item selected.
        /// @param text Search text
        void Search(const QString &text);

        /// Updates possible actions depending on the currently active tree view item.
        void UpdateActions();

        /// Opens download progress dialog.
        /// @param asset_id Asset id.
        /// @param name Asset name.
        void OpenDownloadProgess(const QString &asset_id, const QString &name);

        /// Aborts download.
        /// @param asset_id Asset id.
        void AbortDownload(const QString &asset_id);

        /// Finish download progress notification
        /// @param item id
        void FinishProgessNotification(const QString &id);

        /// Creates new progress notification for file upload.
        /// @param filename Filename.
        void UploadStarted(const QString &filename);

        /// Shows failure notification.
        /// @param filename Filename.
        /// @param reason Reason of failure.
        void UploadFailed(const QString &filename, const QString &reason);

        //! Sending notification from DL/UL thread, cant create widgets there
        //! because its not the main ui thread
        void CreateNotification(QString message, int hide_time);

    private:
        Q_DISABLE_COPY(InventoryWindow);

        /// Initializes the inventory UI.
        void InitInventoryWindow();

        /// Creates the context menu actions.
        void CreateActions();

        /// Inventory view model.
        QPointer<InventoryItemModel> inventoryItemModel_;

        /// Inventory window widget.
        QWidget *mainWidget_;

        /// Layout 
        QVBoxLayout *layout_;

        // Inventory tree view widget
        InventoryTreeView *treeView_;

        // Inventory search field
        QLineEdit *lineEditSearch_;

        /// Map of active download progress dialogs.
//        QMap<QString, QMessageBox *> downloadDialogs_;

        /// Offset for download dialog positions.
//        size_t offset_;

        /// Last file path used when using open file dialog.
        QString lastUsedPath_;

        /// Used to follow and update ongoing downloads and uploads
        QMap<QString, UiServices::ProgressController*> notification_progress_map_;
    };



}

#endif
