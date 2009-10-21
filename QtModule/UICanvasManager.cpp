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

	void UICanvasManager::AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle)
	{
		if (controlBarLayout_)
		{	
			ControlBarButton *button = new ControlBarButton(controlBarWidget_, canvas, buttonTitle);
			controlBarLayout_->addWidget(button);
			QSize widgetSize = controlBarWidget_->size();
			controlBarCanvas_->SetCanvasSize(widgetSize.width()+button->size().width(), widgetSize.height());
		}
	}

	bool UICanvasManager::RemoveCanvasFromControlBar(boost::shared_ptr<QtUI::UICanvas> canvas)
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
			controlBarWidget_->setGeometry(0,0,0,0);
			controlBarLayout_ = controlBarWidget_->findChild<QHBoxLayout *>("layout_ControlBar");

			controlBarCanvas_ = spQtModule->CreateCanvas(UICanvas::Internal).lock();
			controlBarCanvas_->SetPosition(0,0);
			controlBarCanvas_->SetCanvasSize(25, 27);
			controlBarCanvas_->SetLockPosition(true);
			controlBarCanvas_->SetCanvasResizeLock(true);
			controlBarCanvas_->SetTop();
			controlBarCanvas_->AddWidget(controlBarWidget_);			
			controlBarCanvas_->Show();
		}
	}

	ControlBarButton::ControlBarButton(QWidget *parent, boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle)
		: QPushButton(buttonTitle, parent), myCanvas_(canvas), myTitle_(buttonTitle)
	{
		QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::PushButton);
		this->setMinimumSize(QSize(25,15));
		this->setSizePolicy(policy);
		this->setFlat(true);
		this->resize(QSize(70,15));
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