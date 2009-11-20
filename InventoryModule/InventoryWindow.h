// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryWindow.h
 *  @brief The inventory window.
 */

#ifndef incl_InventoryModule_InventoryWindow_h
#define incl_InventoryModule_InventoryWindow_h

#include "InventoryModule.h"
#include "RexTypes.h"

#include <QObject>
#include <QPointer>

class QWidget;
class QPushButton;
class QTreeWidgetItem;
class QTreeView;
class QItemSelection;
class QModelIndex;

namespace QtUI
{
    class QtModule;
    class UICanvas;
}

namespace Foundation
{
    class Framework;
    class EventDataInterface;
}

namespace Inventory
{
    class InventoryItemEventData;
    class InventoryItemModel;
    class AbstractInventoryDataModel;

    class InventoryWindow : public QObject
    {
        Q_OBJECT

        MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { const std::string &name = "InventoryModule::UI"; return name; }

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        InventoryWindow(Foundation::Framework *framework);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        /// Initializes the OpenSim inventory data/view model.
        /// @param world_stream
        /// @return Pointer to inventory data model.
        InventoryPtr InitOpenSimInventoryTreeModel(ProtocolUtilities::WorldStreamPtr world_stream);

        /// Initializes the Taiga WebDAV data/view model.
        /// @param identityUrl
        /// @param hostUrl
        /// @return Pointer to inventory data model.
        InventoryPtr InitWebDavInventoryTreeModel(const std::string &identityUrl, const std::string &hostUrl);

        /// Resets the inventory tree model.
        void ResetInventoryTreeModel();

        /// Toggles inventory window visibility.
        void Toggle();

        /// Hides the inventory window.
        void Hide();

        /// Handles data from InventoryDescendent packet.
        void HandleInventoryDescendent(InventoryItemEventData *item_data);

    private slots:
        /// Expands the inventory folder in the treeview
        /// @param index Index of folder to be expanded.
        void ExpandFolder(const QModelIndex &index);

        /// Adds new folder.
        void AddFolder();

        /// Deletes item (folder/asset).
        void DeleteItem();

        /// Renames item (folder/asset).
        void RenameItem();

        /// File upload.
        void Upload();

        /// File download.
        void Download();

        /// Updates possible actions depending on the currently active tree view item.
        void UpdateActions();

    private:
        Q_DISABLE_COPY(InventoryWindow);

        /// Initializes the inventory UI.
        void InitInventoryWindow();

        /// Type of the present inventory model
        InventoryModule::InventoryDataModelType inventoryType_;

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// QtModule pointer.
        boost::shared_ptr<QtUI::QtModule> qtModule_;

        /// Canvas for the inventory window.
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        /// Inventory view model.
        QPointer<InventoryItemModel> inventoryItemModel_;

        /// Inventory window widget.
        QWidget *inventoryWidget_;

        /// Treeview widget.
        QTreeView *treeView_;

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
    };
}

#endif
