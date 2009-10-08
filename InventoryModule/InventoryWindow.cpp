// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "OpenSimInventoryDataModel.h"
#include "InventoryViewModel.h"
#include "QtModule.h"
#include "RexLogicModule.h"

#include <QtUiTools>
#include <QFile>

namespace Inventory
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogic::RexLogicModule *rexLogic) :
    framework_(framework), rexLogicModule_(rexLogic), inventoryWidget_(0), viewModel_(0), treeView_(0),
    buttonClose_(0), buttonDownload_(0), buttonUpload_(0), buttonAddFolder_(0), buttonDeleteFolder_(0), buttonRename_(0)
{
    // Get QtModule and create canvas
    qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
    if (!qtModule_.get())
        return;

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();

    // Init Inventory Widget and connect close signal
    InitInventoryWindow();

    QObject::connect(buttonClose_, SIGNAL(clicked()), this, SLOT(CloseInventoryWindow()));

    // Add local widget to canvas, setup initial size and title and show canvas
    canvas_->SetCanvasSize(300, 275);
    canvas_->SetCanvasWindowTitle(QString("Inventory"));
    canvas_->AddWidget(inventoryWidget_);
}

// virtual
InventoryWindow::~InventoryWindow()
{
    SAFE_DELETE(viewModel_);
}

void InventoryWindow::Show()
{
    if (canvas_)
        canvas_->Show();
}

void InventoryWindow::Hide()
{
    if (canvas_)
        canvas_->Hide();
}

void InventoryWindow::InitOpenSimInventoryTreeModel()
{
    OpenSimInventoryDataModel dataModel(rexLogicModule_->GetInventory().get());
//    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(dataModel.GetFirstChildFolderByName("TEST"));
    //OpenSimInventoryDataModel dataModel = CreateOpenSimDataModel(InventorySkeleton *inventory);
//    viewModel_ = new InventoryViewModel(dataModel);
    treeView_->setModel(0);
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(viewModel_);
}

void InventoryWindow::CloseInventoryWindow()
{
    if (qtModule_.get() != 0)
    {
        canvas_->Hide();
        qtModule_.get()->DeleteCanvas(canvas_);
    }
}

void InventoryWindow::InitInventoryWindow()
{
    // Create widget from ui file
    QUiLoader loader;
    QFile uiFile("./data/ui/inventory_main.ui");
    inventoryWidget_ = loader.load(&uiFile, 0);
    uiFile.close();

    // Get controls
    buttonClose_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Close");
    buttonDownload_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Download");
    buttonUpload_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Upload");
    buttonAddFolder_ = inventoryWidget_->findChild<QPushButton*>("pushButton_AddFolder");
    buttonDeleteFolder_ = inventoryWidget_->findChild<QPushButton*>("pushButton_DeleteFolder");
    buttonRename_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Rename");
    treeView_ = inventoryWidget_->findChild<QTreeView*>("treeView");
}

} // namespace Inventory
