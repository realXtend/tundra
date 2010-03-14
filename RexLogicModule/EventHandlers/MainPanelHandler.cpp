// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MainPanelHandler.h"
#include "ModuleManager.h"

#include "Framework.h"
#include "WorldStream.h"

#include <boost/shared_ptr.hpp>

#include <UiModule.h>
#include <Inworld/InworldSceneController.h>
#include <Inworld/View/MainPanel.h>

namespace RexLogic
{
    MainPanelHandler::MainPanelHandler(Foundation::Framework *framework, RexLogicModule *rex_logic_module)
        : QObject(),
          framework_(framework),
          rex_logic_module_(rex_logic_module)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>
            (Foundation::Module::MT_UiServices).lock();

        if (ui_module.get())
        {
            CoreUi::MainPanel *main_panel = ui_module->GetInworldSceneController()->GetMainPanel();
            connect(main_panel->buttonLogout, SIGNAL( clicked() ), this, SLOT( LogoutRequested() ));
            connect(main_panel->buttonQuit, SIGNAL( clicked() ), this, SLOT( QuitRequested() ));
        }
    }

    MainPanelHandler::~MainPanelHandler()
    {
    }

    void MainPanelHandler::LogoutRequested()
    {
        rex_logic_module_->LogoutAndDeleteWorld();
    }

    void MainPanelHandler::QuitRequested()
    {
        if (rex_logic_module_->GetServerConnection()->IsConnected())
            rex_logic_module_->LogoutAndDeleteWorld();

        framework_->Exit();
    }
}
