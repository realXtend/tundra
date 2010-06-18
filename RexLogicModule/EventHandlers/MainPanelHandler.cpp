// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MainPanelHandler.h"
#include "RexLogicModule.h"

#include "ModuleManager.h"
#include "Framework.h"
#include "WorldStream.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Common/UiAction.h"

#include <boost/shared_ptr.hpp>

namespace RexLogic
{

MainPanelHandler::MainPanelHandler(RexLogicModule *rex_logic_module) :
    QObject(), rex_logic_module_(rex_logic_module)
{
    UiModulePtr ui_module = rex_logic_module_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(
        ).lock();
    if (ui_module.get())
    {
        UiServices::UiAction *quit_action = new UiServices::UiAction(this);
        connect(quit_action, SIGNAL(triggered()), SLOT(QuitRequested()));

        ui_module->GetInworldSceneController()->GetControlPanelManager()->SetHandler(UiDefines::Quit, quit_action);
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
    rex_logic_module_->GetFramework()->Exit();
}

}
