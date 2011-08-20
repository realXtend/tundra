// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MainPanelHandler.h"
#include "RexLogicModule.h"

#include "ModuleManager.h"
#include "Framework.h"
#include "WorldStream.h"

#ifdef USE_UIMODULE
#include "Common/UiAction.h"
#include "UiServiceInterface.h"
#include <QPushButton>
#endif

namespace RexLogic
{

MainPanelHandler::MainPanelHandler(RexLogicModule *rexlogic) : rexlogic_(rexlogic)
{
#ifdef USE_UIMODULE
    //Do it with QPushButton
    UiServiceInterface *ui = rexlogic_->GetFramework()->UiService();
    if (ui)
    {
        QPushButton *butt = new QPushButton("Quit");
        connect(butt, SIGNAL(clicked(bool)),SLOT(QuitRequested()));
        ui->AddAnchoredWidgetToScene(butt, Qt::TopRightCorner, Qt::Horizontal, 1, true);
    }
    

    
    /*
    UiServices::UiModule *ui_module = rexlogic_->GetFramework()->GetModule<UiServices::UiModule>();
    if (ui_module)
    {
        UiServices::UiAction *quit_action = new UiServices::UiAction(this);
        connect(quit_action, SIGNAL(triggered()), SLOT(QuitRequested()));

        UiServices::InworldSceneController * scene_controller = ui_module->GetInworldSceneController();

		if (scene_controller)
			scene_controller->GetControlPanelManager()->SetHandler(UiServices::Quit, quit_action);
    }
     */
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
