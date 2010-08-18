// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MainPanelHandler.h"
#include "RexLogicModule.h"

#include "ModuleManager.h"
#include "Framework.h"
#include "WorldStream.h"

#ifndef UISERVICE_TEST
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Common/UiAction.h"
#endif

namespace RexLogic
{

MainPanelHandler::MainPanelHandler(RexLogicModule *rexlogic) : rexlogic_(rexlogic)
{
#ifndef UISERVICE_TEST
    UiServices::UiModule *ui_module = rexlogic_->GetFramework()->GetModule<UiServices::UiModule>();
    if (ui_module)
    {
        UiServices::UiAction *quit_action = new UiServices::UiAction(this);
        connect(quit_action, SIGNAL(triggered()), SLOT(QuitRequested()));

        ui_module->GetInworldSceneController()->GetControlPanelManager()->SetHandler(UiServices::Quit, quit_action);
    }
#endif
}

MainPanelHandler::~MainPanelHandler()
{
}

void MainPanelHandler::LogoutRequested()
{
    rexlogic_->LogoutAndDeleteWorld();
}

void MainPanelHandler::QuitRequested()
{
    if (rexlogic_->GetServerConnection()->IsConnected())
        rexlogic_->LogoutAndDeleteWorld();
    rexlogic_->GetFramework()->Exit();
}

}
