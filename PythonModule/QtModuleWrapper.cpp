#include "QtModuleWrapper.h"

namespace PythonScript {

    QtModuleWrapper::QtModuleWrapper(Foundation::Framework *framework)
    {
        framework_ = framework;
    }


    QtUI::UICanvas* QtModuleWrapper::CreateCanvas(int mode) {
        boost::shared_ptr<QtUI::QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui != 0)
        {
            if (mode==1)
                canvas_ = qt_ui->CreateCanvas(QtUI::UICanvas::Internal).lock();
            else
                canvas_ = qt_ui->CreateCanvas(QtUI::UICanvas::External).lock();
            
            QtUI::UICanvas* qcanvas = canvas_.get();
            return qcanvas;
        }
        return NULL;
        
    }

    void QtModuleWrapper::DeleteCanvas(QtUI::UICanvas* canvas) {
        boost::shared_ptr<QtUI::QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
      
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui != 0)
        {
            //boost::shared_ptr<QtUI::UICanvas> canvas_(canvas);
            //canvas_->Hide();
			canvas->Hide();
			qt_ui->DeleteCanvas(canvas->GetID());
        }
    }
}