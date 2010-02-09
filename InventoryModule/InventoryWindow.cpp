// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryWindow.cpp
 *  @brief  Inventory window. Should be totally unaware of the underlaying inventory data model.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InventoryWindow.h"
#include "InventoryModule.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "InventoryTreeView.h"

#include "Framework.h"
#include "RexLogicModule.h"
#include "Inventory/InventoryEvents.h"
#include "QtUtils.h"

#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>
//#include <DirectoryView.h>

#include <QUiLoader>
#include <QFile>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include "MemoryLeakCheck.h"

namespace Inventory
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework) :
    framework_(framework),
    inventoryWidget_(0),
    inventoryItemModel_(0),
    treeView_(0),
    actionMenu_(0),
    actionDelete_(0),
    actionRename_(0),
    actionCut_(0),
    actionPaste_(0),
    actionNewFolder_(0),
    actionOpen_(0),
    actionCopyAssetReference_(0),
    actionUpload_(0),
    actionDownload_(0),
    actionSeparator_(0),
//    offset_(0),
    lastUsedPath_(QDir::currentPath()),
    uploadCount_(0),
    uploadWidget_(0)
{
    InitInventoryWindow();
}

// virtual
InventoryWindow::~InventoryWindow()
{
    SAFE_DELETE(inventoryItemModel_);
    SAFE_DELETE(treeView_);
    SAFE_DELETE(inventoryWidget_);
    SAFE_DELETE(actionMenu_);
    SAFE_DELETE(actionDelete_);
    SAFE_DELETE(actionRename_);
    SAFE_DELETE(actionCut_);
    SAFE_DELETE(actionPaste_);
    SAFE_DELETE(actionNewFolder_);
    SAFE_DELETE(actionOpen_);
    SAFE_DELETE(actionCopyAssetReference_);
    SAFE_DELETE(actionUpload_);
    SAFE_DELETE(actionDownload_);
    SAFE_DELETE(actionSeparator_);
    SAFE_DELETE(uploadWidget_);

    QMutableMapIterator<QString, QMessageBox *> it(downloadDialogs_);
    while(it.hasNext())
    {
        QMessageBox *msgBox = it.next().value();
        SAFE_DELETE(msgBox);
        it.remove();
    }
}

void InventoryWindow::Hide()
{
    proxyWidget_->hide();
}

void InventoryWindow::InitInventoryTreeModel(InventoryPtr inventory_model)
{
    if (inventoryItemModel_)
    {
        InventoryModule::LogError("Inventory treeview has already item model set!");
        return;
    }

    // Create inventory item model.
    inventoryItemModel_ = new InventoryItemModel(inventory_model.get());
    treeView_->setModel(inventoryItemModel_);

    // Connect download progress signals.
    QObject::connect(inventory_model.get(), SIGNAL(DownloadStarted(const QString &, const QString &)),
        this, SLOT(OpenDownloadProgess(const QString &, const QString &)));

    QObject::connect(inventory_model.get(), SIGNAL(DownloadAborted(const QString &)),
        this, SLOT(AbortDownload(const QString &)));

    QObject::connect(inventory_model.get(), SIGNAL(DownloadCompleted(const QString &)),
        this, SLOT(CloseDownloadProgess(const QString &)));

    // Connect upload progress signals.
    QObject::connect(inventory_model.get(), SIGNAL(MultiUploadStarted(size_t)),
        this, SLOT(OpenUploadProgress(size_t)));

    QObject::connect(inventory_model.get(), SIGNAL(UploadStarted(const QString &)),
        this, SLOT(UploadStarted(const QString &)));

//    QObject::connect(inventory_model.get(), SIGNAL(UploadFailed(const QString &)),
//        this, SLOT(UploadProgress(const QString &)));

//    QObject::connect(inventory_model.get(), SIGNAL(UploadCompleted(const QString &)),
//        this, SLOT(UploadProgress(const QString &)));

    QObject::connect(inventory_model.get(), SIGNAL(MultiUploadCompleted()),
        this, SLOT(CloseUploadProgress()));

    // Connect selectionChanged
    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(inventoryItemModel_);
}

void InventoryWindow::OpenItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();

    inventoryItemModel_->Open(index);

    treeView_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::AddFolder()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    inventoryItemModel_->Open(index);

    if (!inventoryItemModel_->InsertFolder(index.row(), index, "New Folder"))
        return;

    treeView_->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();

    RenameItem();
}

void InventoryWindow::DeleteItem()
{
    ///\todo Delete for multiple items.
/*
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;
*/

    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    if (model->removeRow(index.row(), index.parent()))
        UpdateActions();
}

void InventoryWindow::RenameItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();

    if (treeView_->model()->flags(index) & Qt::ItemIsEditable)
        treeView_->edit(index);
}

void InventoryWindow::Upload()
{
//    QtUI::DirectoryView *dv = new QtUI::DirectoryView(this, SLOT(UploadFiles(QStringList &)), 0);

    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QStringList filenames = Foundation::QtUtils::GetOpenRexFilenames(lastUsedPath_.toStdString());
    if (filenames.empty())
        return;

    QString path = filenames.last();
    path.remove(path.lastIndexOf('/'), path.size() - path.lastIndexOf('/') + 1);
    QDir dir(path);
    if (dir.exists())
        lastUsedPath_ = path;

    QStringList itemNames;
    inventoryItemModel_->Upload(index, filenames, itemNames);
}

void InventoryWindow::UploadFiles(QStringList &filenames)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (filenames.isEmpty())
        return;

    QStringList itemNames;
    inventoryItemModel_->Upload(index, filenames, itemNames);
}

void InventoryWindow::Download()
{
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QString storePath = QFileDialog::getExistingDirectory(inventoryWidget_, "Select location for file download", lastUsedPath_);
    if (storePath.isEmpty())
        return;

    inventoryItemModel_->Download(storePath, selection);
}

void InventoryWindow::CopyAssetReference()
{
    inventoryItemModel_->CopyAssetReferenceToClipboard(treeView_->selectionModel()->currentIndex());
}

void InventoryWindow::UpdateActions()
{
    ///\todo Cut, Copy, Paste: actionCut_, actionCopy_, actionCut_

    QModelIndex index = treeView_->selectionModel()->currentIndex();

    bool canAddFolder = !inventoryItemModel_->IsLibraryItem(index);
    actionNewFolder_->setEnabled(canAddFolder);

    bool editable = treeView_->model()->flags(index) & Qt::ItemIsEditable;
    actionDelete_->setEnabled(editable);
    actionRename_->setEnabled(editable);

    bool is_asset = inventoryItemModel_->GetItemType(index) == AbstractInventoryItem::Type_Asset;
    actionCopyAssetReference_->setEnabled(is_asset);
    actionOpen_->setEnabled(is_asset);

    bool hasCurrent = treeView_->selectionModel()->currentIndex().isValid();

    if (hasCurrent)
        treeView_->closePersistentEditor(treeView_->selectionModel()->currentIndex());
}

void InventoryWindow::OpenDownloadProgess(const QString &asset_id, const QString &name)
{
    // Make one custom widget as download manager, untill then disable this because its quite intrusive for a user to pop up dialogs
    // and they dont atm come to from and the position is calculated with inv window or something strange

/*
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, "", "Downloading asset " + asset_id, QMessageBox::Ok);
    msgBox->setModal(false);
    downloadDialogs_[asset_id] = msgBox;

    boost::shared_ptr<UiServices::UiModule> ui_module =
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (ui_module.get())
    {
        QPointF pos = inventoryWidget_->mapToGlobal(QPoint(0, 0));
        pos.setX(pos.x() + offset_);
        pos.setY(pos.y() + offset_);
        offset_ += 20;

        ui_module->GetSceneManager()->AddWidgetToScene(
            msgBox, UiServices::UiWidgetProperties(pos, msgBox->size(), Qt::Dialog, "Download: " + name, false));
    }

    msgBox->show();
*/

    emit Notification(QString("Downloading %1").arg(name), 9000);
}

void InventoryWindow::AbortDownload(const QString &asset_id)
{
    ///\todo
}

void InventoryWindow::CloseDownloadProgess(const QString &asset_id)
{
/*
    QMessageBox *msgBox = downloadDialogs_.take(asset_id);
    if (msgBox)
        delete msgBox;
*/
}

void InventoryWindow::OpenUploadProgress(size_t file_count)
{
    QProgressBar *progressBar = uploadWidget_->findChild<QProgressBar *>("progressBar");
    progressBar->setRange(0, file_count);
    progressBar->setValue(uploadCount_);
    uploadProxyWidget_->show();
}

void InventoryWindow::UploadStarted(const QString &filename)
{
    QProgressBar *progressBar = uploadWidget_->findChild<QProgressBar *>("progressBar");
    QLabel *labelFileNumber = uploadWidget_->findChild<QLabel *>("labelFileNumber");

    ++uploadCount_;
    int max_value = progressBar->maximum();
    if (uploadCount_ <= max_value)
    {
        progressBar->setValue(uploadCount_);
        labelFileNumber->setText(QString("%1 (%2/%3)").arg(filename).arg(uploadCount_).arg(max_value));
    }
}

/*
void InventoryWindow::UploadCompleted(const QString &filename)
{
}
*/

void InventoryWindow::CloseUploadProgress()
{
    QProgressBar *progressBar = uploadWidget_->findChild<QProgressBar *>("progressBar");
    progressBar->reset();
    uploadCount_ = 0;
    uploadProxyWidget_->hide();
}

void InventoryWindow::InitInventoryWindow()
{
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    QUiLoader loader;
    QFile uiFile("./data/ui/inventory.ui");
    inventoryWidget_ = loader.load(&uiFile, 0);
    uiFile.close();

    // Create inventory tree view.
    treeView_ = new InventoryTreeView;
    QHBoxLayout *hlayout = inventoryWidget_->findChild<QHBoxLayout *>("horizontalLayout_BottomContainer");
    hlayout->addWidget(treeView_);

    // Connect signals
    ///\todo Connecting both these signals causes WebDav inventory to work incorrectly.
//    QObject::connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));
    QObject::connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OpenItem()));

    proxyWidget_ = ui_module->GetSceneManager()->AddWidgetToScene(
        inventoryWidget_, UiServices::UiWidgetProperties("Inventory", UiServices::ModuleWidget));

    // Upload progress window
    QFile file("./data/ui/uploadprogress.ui");
    uploadWidget_ = loader.load(&file, 0);
    file.close();

    UiServices::UiWidgetProperties widget_properties("Upload", UiServices::SceneWidget);
    widget_properties.SetPosition(inventoryWidget_->mapToGlobal(QPoint(0, 0)));
    uploadProxyWidget_ = ui_module->GetSceneManager()->AddWidgetToScene(uploadWidget_, widget_properties);

    connect(this, SIGNAL(Notification(const QString &, int)), ui_module->GetNotificationManager(),SLOT(ShowInformationString(const QString &, int)));

    CreateActions();
}

void InventoryWindow::CreateActions()
{
/*
    itemMenu_ = new QMenu(this);
    fileTransferMenu_ = new QMenu(this);
    QObject::connect(actionMenu_, SIGNAL(aboutToShow()), this, SLOT(UpdateActions()));
*/
    // File transfer actions
    actionUpload_= new QAction(tr("&Upload"), this);
    actionUpload_->setStatusTip(tr("Upload file to your inventory"));
    QObject::connect(actionUpload_, SIGNAL(triggered()), this, SLOT(Upload()));
    treeView_->addAction(actionUpload_);

    actionDownload_= new QAction(tr("&Download"), this);
    actionDownload_->setStatusTip(tr("Download assets to your hard drive"));
    QObject::connect(actionDownload_, SIGNAL(triggered()), this, SLOT(Download()));
    treeView_->addAction(actionDownload_);

    // Add separator
    actionSeparator_= new QAction(this);
    actionSeparator_->setSeparator(true);
    treeView_->addAction(actionSeparator_);

    // Inventory item actions.
    actionDelete_ = new QAction(tr("&Delete"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Delete this item"));
    QObject::connect(actionDelete_, SIGNAL(triggered()), this, SLOT(DeleteItem()));
    treeView_->addAction(actionDelete_);

    actionRename_ = new QAction(tr("&Rename"), this);
    //actionRename_->setShortcuts();
    actionRename_->setStatusTip(tr("Rename this item"));
    QObject::connect(actionRename_, SIGNAL(triggered()), this, SLOT(RenameItem()));
    treeView_->addAction(actionRename_);

/*
    actionCut_ = new QAction(tr("&Cut"), this);
    actionDelete_->setShortcuts(QKeySequence::Cut);
    actionDelete_->setStatusTip(tr("Cut this item"));
    QObject::connect(actionCut_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionCut_);

    actionPaste_ = new QAction(tr("&Paste"), this);
    actionDelete_->setShortcuts(QKeySequence::Paste);
    actionDelete_->setStatusTip(tr("Paste this item"));
    QObject::connect(actionPaste_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionPaste_);
*/

    actionNewFolder_ = new QAction(tr("&New folder"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionNewFolder_->setStatusTip(tr("Create new folder"));
    QObject::connect(actionNewFolder_, SIGNAL(triggered()), this, SLOT(AddFolder()));
    treeView_->addAction(actionNewFolder_);

    actionOpen_ = new QAction(tr("&Open"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionOpen_->setStatusTip(tr("Open this item"));
    QObject::connect(actionOpen_, SIGNAL(triggered()), this, SLOT(OpenItem()));
    treeView_->addAction(actionOpen_);

    actionCopyAssetReference_ = new QAction(tr("&Copy asset reference"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionCopyAssetReference_->setStatusTip(tr("Delete this item"));
    QObject::connect(actionCopyAssetReference_, SIGNAL(triggered()), this, SLOT(CopyAssetReference()));
    treeView_->addAction(actionCopyAssetReference_);
}

} // namespace Inventory
