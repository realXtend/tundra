// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UICanvasManager.h"
#include "QtModule.h"

#include <QFile>
#include <QtUiTools>

namespace QtUI
{
	UICanvasManager::UICanvasManager(Foundation::Framework *framework)
		: framework_(framework), controlBarWidget_(0), controlBarLayout_(0)
	{
		qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui);
		if (qtModule_.lock().get())
			InitManagedWidgets();
	}

	UICanvasManager::~UICanvasManager()
	{

	}

	const void UICanvasManager::AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, QString buttonTitle)
	{
		if (controlBarLayout_)
		{
			controlBarLayout_->addWidget(new ControlBarButton(controlBarWidget_, canvas, buttonTitle));
		}
	}

	const bool UICanvasManager::RemoveCanvasFromControlBar(boost::shared_ptr<QtUI::UICanvas> canvas)
	{
		if (controlBarLayout_)
		{
			return false;
		}
		else
			return false;
	}

	void UICanvasManager::InitManagedWidgets()
	{
		boost::shared_ptr<QtUI::QtModule> spQtModule = qtModule_.lock();
		if ( spQtModule.get() )
		{
			QUiLoader loader;
			QFile uiFile("./data/ui/controlbar.ui");
			controlBarWidget_ = loader.load(&uiFile);
			controlBarWidget_->setGeometry(0,0,200,27);
			controlBarLayout_ = controlBarWidget_->findChild<QHBoxLayout *>("layout_ControlBar");

			controlBarCanvas_ = spQtModule->CreateCanvas(UICanvas::Internal).lock();
			controlBarCanvas_->AddWidget(controlBarWidget_);
			controlBarCanvas_->SetPosition(0,0);
			controlBarCanvas_->SetCanvasSize(200,27);
			controlBarCanvas_->SetLockPosition(true);
			controlBarCanvas_->SetCanvasResizeLock(true);
			controlBarCanvas_->SetTop();
			controlBarCanvas_->Show();
		}
	}

	ControlBarButton::ControlBarButton(QWidget *parent, boost::shared_ptr<QtUI::UICanvas> canvas, const QString buttonTitle)
		: QPushButton(buttonTitle, parent), myCanvas_(canvas), myTitle_(buttonTitle)
	{
		this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		this->setFlat(true);
		QObject::connect(this, SIGNAL( clicked() ), this, SLOT( toggleShow() )); 
	}

	void ControlBarButton::toggleShow()
	{
		myCanvas_->Render();
		if (myCanvas_->isHidden())
			myCanvas_->Show();
		else
			myCanvas_->Hide();
	}

	boost::shared_ptr<QtUI::UICanvas> ControlBarButton::GetCanvas()
	{
		return myCanvas_;
	}
}