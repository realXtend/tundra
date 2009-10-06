// For conditions of distribution and use, see copyright notice in license.txt

#include "QtModule.h"

#include <QtUiTools>
#include <QFile>

#include "InventoryWindow.h"

namespace Inventory
{
	InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogic::RexLogicModule *rexLogic)
		: framework_(framework), rexLogicModule_(rexLogic)
	{
		// Get QtModule and create canvas
		qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
		if ( qtModule_.get() == 0)
			return;
		canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::External).lock();
		// Init Inventory Widget and connect close signal
		InitInventoryWindow();
		QObject::connect(buttonClose_, SIGNAL( clicked() ), this, SLOT( CloseInventoryWindow() ));
		// Add local widget to canvas, setup initial size and title and show canvas
		canvas_->SetCanvasSize(300, 275);
		canvas_->SetCanvasWindowTitle(QString("Inventory"));
		canvas_->AddWidget(inventoryWidget_);
		canvas_->Show();
	}

	InventoryWindow::~InventoryWindow()
	{

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
}