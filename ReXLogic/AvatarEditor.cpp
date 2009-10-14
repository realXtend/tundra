// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditor.h"
#include "Avatar.h"
#include "RexLogicModule.h"
#include "QtModule.h"
#include "QtUtils.h"

#include <QtUiTools>
#include <QPushButton>

namespace RexLogic
{

    AvatarEditor::AvatarEditor(RexLogicModule* rexlogicmodule) :
        rexlogicmodule_(rexlogicmodule)
    {
        InitEditorWindow();
    }

    AvatarEditor::~AvatarEditor()
    {
        avatar_widget_ = 0;

        Foundation::ModuleSharedPtr qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui)
        {
            if (canvas_)
                qt_ui->DeleteCanvas(canvas_->GetID());
        }
    }

    void AvatarEditor::Toggle()
    {
        if (canvas_)
        {
            if (canvas_->IsHidden())
                canvas_->Show();
            else
                canvas_->Hide();
        }
    }
    
    void AvatarEditor::Close()
    {
        // Actually just hide the canvas
        if (canvas_)
            canvas_->Hide();
    }
    
    void AvatarEditor::ExportAvatar()
    {
        rexlogicmodule_->GetAvatarHandler()->ExportUserAvatar();
    }
    
    void AvatarEditor::LoadAvatar()
    {
        const std::string filter = "Avatar description files (*.xml);;Avatar mesh files (*.mesh)";
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Choose avatar file", Foundation::QtUtils::GetCurrentPath());        

        if (!filename.empty())
            rexlogicmodule_->GetAvatarHandler()->LoadUserAvatarFromFile(filename);                        
    }
    
    void AvatarEditor::RevertAvatar()
    {
        // Reload avatar from storage, or reload default
        rexlogicmodule_->GetAvatarHandler()->ReloadUserAvatar();        
    }
    
    void AvatarEditor::InitEditorWindow()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

        // If this occurs, we're most probably operating in headless mode.
        if ( qt_module.get() == 0)
            return;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();

        QUiLoader loader;
        QFile file("./data/ui/avatareditor.ui");

        if (!file.exists())
        {
            RexLogicModule::LogError("Cannot find avatar editor .ui file.");
            return;
        }

        avatar_widget_ = loader.load(&file); 
        if (!avatar_widget_)
            return;
            
        // Set canvas size. 
        QSize size = avatar_widget_->size();
        canvas_->SetCanvasSize(size.width(), size.height());
        canvas_->SetCanvasWindowTitle(QString("Avatar Editor"));

        canvas_->AddWidget(avatar_widget_);
        //canvas_->Show();
        
        // Connect signals            
        QPushButton *button = avatar_widget_->findChild<QPushButton *>("but_export");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(ExportAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_load");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(LoadAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_revert");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(RevertAvatar()));
    }
}