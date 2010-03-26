// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UICanvasManager.h"
#include "QtModule.h"

#include <QFile>
#include <QUiLoader>

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

    void UICanvasManager::SetVisible(bool show)
    {
        if (controlBarCanvas_.get())
        {
            if (show)
                controlBarCanvas_->Show();
            else
                controlBarCanvas_->Hide();
        }
    }

    void UICanvasManager::AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle)
    {
        if (controlBarLayout_)
        {    
            ControlBarButton *button = new ControlBarButton(controlBarWidget_, canvas, buttonTitle);
            controlBarLayout_->addWidget(button);
            QSize widgetSize = controlBarWidget_->size();
            controlBarCanvas_->SetSize(widgetSize.width()+button->size().width(), widgetSize.height());
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

    bool UICanvasManager::RemoveCanvasFromControlBar(const QString& id)
    {
        if (controlBarLayout_)
        {
            for(int index = 0; index < controlBarLayout_->count(); index++)
            {
                QLayoutItem* temp = controlBarLayout_->itemAt(index);
                ControlBarButton* button = (ControlBarButton *)temp->widget();
                
                if (button)
                {
                    if (button->GetCanvas())
                    {
                        if (button->GetCanvas()->GetID() == id) 
                        {
                            controlBarLayout_->takeAt(index);
                            controlBarCanvas_->SetSize(controlBarCanvas_->GetSize().width()-button->size().width(), controlBarCanvas_->GetSize().height());
                            delete button;
                            button = 0;
                            return true;
                        }
                    }
                }
            }
        }
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
            controlBarWidget_->setGeometry(0,0,0,25);
            controlBarLayout_ = controlBarWidget_->findChild<QHBoxLayout *>("layout_ControlBar");

            controlBarCanvas_ = spQtModule->CreateCanvas(UICanvas::Internal).lock();
            controlBarCanvas_->SetPosition(0,0);
            controlBarCanvas_->SetSize(25, 25);
            controlBarCanvas_->SetStationary(true);
            controlBarCanvas_->SetResizable(false);
            controlBarCanvas_->SetAlwaysOnTop(true);
            controlBarCanvas_->AddWidget(controlBarWidget_);
            controlBarCanvas_->Hide();
        }
    }

    ControlBarButton::ControlBarButton(QWidget *parent, boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle)
        : QPushButton(buttonTitle, parent), myCanvas_(canvas), myTitle_(buttonTitle)
    {
        QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::PushButton);
        this->setMinimumSize(QSize(25,15));
        this->setSizePolicy(policy);
        this->setFlat(true);
        // Still having some issue with sizing, should go without a layout maybe... 
        // Button min width is 100 so looks ungly if title is short like "qt"
        //this->resize(QSize(70,15)); 
        QObject::connect(this, SIGNAL( clicked() ), this, SLOT( toggleShow() )); 
    }

    void ControlBarButton::toggleShow()
    {
//        myCanvas_->Render();
        if (myCanvas_->IsHidden())
            myCanvas_->Show();
        else
            myCanvas_->Hide();
    }

    boost::shared_ptr<QtUI::UICanvas> ControlBarButton::GetCanvas()
    {
        return myCanvas_;
    }

   
}
