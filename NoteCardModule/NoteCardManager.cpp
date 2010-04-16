// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NoteCardManager.h"
#include "NoteCardModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "ComponentManager.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"
#include "XMLUtilities.h"
#include "SceneEvents.h"
#include "EventManager.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>

using namespace RexTypes;

namespace NoteCard
{
    NoteCardManager::NoteCardManager(Foundation::Framework* framework) :
        framework_(framework)
    {
        Initialize();
    }
    
    NoteCardManager::~NoteCardManager()
    {
    }
    
    void NoteCardManager::Initialize()
    {
        QUiLoader loader;
        QFile file("./data/ui/notecardmanager.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            NoteCardModule::LogError("Could not load notecard manager layout");
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
            NoteCardModule::LogError("Could not add widget to scene");
    }
}

