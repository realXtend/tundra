// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "RexLogicModule.h"
#include "QtModule.h"
#include "UICanvas.h"
#include "InventoryModel.h"

#include <QFile>
#include <QLabel>
#include <QString>
#include <QtUiTools>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace RexLogic
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogicModule *module) :
framework_(framework), rexLogicModule_(module)
{
    InitInventoryWindow();
}

InventoryWindow::~InventoryWindow()
{
}

void InventoryWindow::InitInventoryWindow()
{
    qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

    // If this occurs, we're most probably operating in headless mode.
    if (qtModule_.get() == 0)
        return;

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::External).lock();

    QUiLoader loader;
    QFile file("./data/ui/inventory.ui");
    if (!file.exists())
    {
        RexLogicModule::LogError ("Can't open login.UI file.");
        return;
    }

    inventoryWidget_ = loader.load(&file); 

    ///\todo Here we have strange and not-so-wanted feature.
    /// If you first addWidget (in Internal-canvas) and then SetCanvasSize() result: only partial window is seen.
    /// Must investigate futher that why this happends.

    // Set canvas size. 
    QSize size = inventoryWidget_->size();
    canvas_->SetCanvasSize(size.width(), size.height());
    canvas_->AddWidget(inventoryWidget_);

    // Create connections.
    QPushButton *button = inventoryWidget_->findChild<QPushButton *>("pushButtonRefresh");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(PopulateInventoryView()));

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QObject::connect(treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
    QObject::connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(entered(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(pressed(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));


/*    QAction *createFolderAction = new QAction(MainWindow);
    createFolderAction->setObjectName(QString::fromUtf8("createFolderAction"));
    QObject::connect(createFolderAction, SIGNAL(triggered()), this, SLOT(CreateFolder()));*/

    canvas_->Show();
}

void InventoryWindow::PopulateInventoryView()
{
    inventory_  =  rexLogicModule_->GetInventory();
    if (!inventory_.get())
        return;

//    InventoryTreeModel *model = new InventoryTreeModel(inventory_.get());
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    treeView->setModel(inventory_.get());
}

void InventoryWindow::OpenFolder(QTreeWidgetItem* item, int column)
{
    std::cout << "jdjhdsj" << std::endl;
}

void InventoryWindow::CloseFolder()
{
}

void InventoryWindow::OpenItem()
{
}

void InventoryWindow::FetchInventoryDescendents(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!inventory_.get())
        return;

    ///\todo Use id instead of the name.
    /*std::string name = index.data().toString().toStdString();
    OpenSimProtocol::InventoryFolder *folder  = inventory_->GetFirstChildFolderByName(name.c_str());
    rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendents(folder->GetID(),
        folder->GetParent()->GetID(), 0 , true, true);
    */
}

void InventoryWindow::UpdateActions()
{
/*
    bool hasSelection = !view->selectionModel()->selection().isEmpty();
    removeRowAction->setEnabled(hasSelection);
    removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = view->selectionModel()->currentIndex().isValid();
    insertRowAction->setEnabled(hasCurrent);
    insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        view->closePersistentEditor(view->selectionModel()->currentIndex());

        int row = view->selectionModel()->currentIndex().row();
        int column = view->selectionModel()->currentIndex().column();
        if (view->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
*/
}

/*void InsertIventoryFolder()
{}

void InsertInventoryItem()
{}*/

/*
void InventoryWindow::CreateFolder()
{
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView->model();
    InventoryTreeModel *inventoryModel = static_cast<InventoryTreeModel *>(treeView->model());

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for(int column = 0; column < model->columnCount(index); ++column)
    {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[No data]"), Qt::EditRole);

        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
//    updateActions();
}
*/

void InventoryWindow::InsertRow()
{
/*
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
    }
*/
}

/*
bool InventoryWindow::RemoveColumn(const QModelIndex &parent)
{
    QAbstractItemModel *model = view->model();
    int column = view->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    bool changed = model->removeColumn(column, parent);

    if (!parent.isValid() && changed)
        updateActions();

    return changed;
}
*/

void InventoryWindow::RemoveRow()
{
/*
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
*/
}

} // namespace RexLogic
