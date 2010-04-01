// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"

#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>

namespace ECEditor
{
    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        framework_(framework),
        contents_(0)
    {
        Initialize();
    }
    
    ECEditorWindow::~ECEditorWindow()
    {
    }
    
    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(contents_);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);
        
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
            ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(contents_->windowTitle(), UiServices::ModuleWidget));
        else
            ECEditorModule::LogError("Could not add widget to scene");
    }
}

 