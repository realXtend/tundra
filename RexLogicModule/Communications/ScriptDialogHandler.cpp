#include "StableHeaders.h"
#include <QString>
#include "Communications/ScriptDialogHandler.h"

#include "Communications/ScriptDialogWidget.h"
#include "ModuleManager.h"
#include "RexLogicModule.h"
#include "WorldStream.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include <Inworld/InworldSceneController.h>

namespace RexLogic
{
    ScriptDialogHandler::ScriptDialogHandler(Foundation::Framework* framework): framework_(framework)
    {
        if (!framework_)
        {
            // todo: LogError: "Cannot initialize ScriptDialogHandler, framework pointer not valid."
            return;
        }
    }

    ScriptDialogHandler::~ScriptDialogHandler()
    {
        while (dialogs_.size() > 0)
        {
            ScriptDialogWidget* widget = dialogs_.takeFirst();
            widget->close();
            SAFE_DELETE(widget);
        }
    }

    void ScriptDialogHandler::Handle(ScriptDialogRequest &request)
    {
        ShowDialog(request);
    }

    void ScriptDialogHandler::ShowDialog(ScriptDialogRequest& request)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if ( !ui_module.get())
        {
            // todo: LogError: "Cannot show ScriptDialogWidget, ui_module pointer not valid."
            return;
        }

        QString widget_name = "Message from object";
        ScriptDialogWidget* widget = new ScriptDialogWidget(request, framework_);
        dialogs_.append(widget);
        connect(widget, SIGNAL( OnClosed(s32, QString)), this, SLOT( OnDialogClosed(s32, QString)) );

        UiServices::UiWidgetProperties widget_properties(widget_name, UiServices::SceneWidget);
        UiServices::UiProxyWidget *proxy_widget = ui_module->GetInworldSceneController()->AddWidgetToScene(widget, widget_properties); // we don't need to store a proxy widget here..

        proxy_widget->show();
    }

    void ScriptDialogHandler::OnDialogClosed(s32 channel, QString answer)
    {
        // If user want't to ignore request we do not sent anything to server
        if (answer.length() > 0)
        {
            boost::shared_ptr<RexLogicModule> rexlogic = framework_->GetModuleManager()->GetModule<RexLogicModule>(Foundation::Module::MT_WorldLogic).lock();
            if (rexlogic.get())
            {
                RexLogic::WorldStreamConnectionPtr connection = rexlogic->GetServerConnection();
                if (connection)
                    connection->SendChatFromViewerPacket(std::string(answer.toUtf8()).c_str(), channel);
            }
        }
    }

} // end of namespace: RexLogic
