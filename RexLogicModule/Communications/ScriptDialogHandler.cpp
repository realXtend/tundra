// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Communications/ScriptDialogHandler.h"
#include "Communications/ScriptDialogWidget.h"
#include "RexLogicModule.h"

#include "ModuleManager.h"
#include "WorldStream.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"

namespace RexLogic
{
    ScriptDialogHandler::ScriptDialogHandler(RexLogicModule *owner): owner_(owner)
    {
        assert(owner_);
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
        Foundation::UiServiceInterface *ui = owner_->GetFramework()->GetService<Foundation::UiServiceInterface>();
        if (!ui)
        {
            RexLogicModule::LogError("Cannot show ScriptDialogWidget, UI service pointer not valid.");
            return;
        }

        ScriptDialogWidget* widget = new ScriptDialogWidget(request);
        dialogs_.append(widget);
        connect(widget, SIGNAL(OnClosed(int, QString)), this, SLOT(OnDialogClosed(int, QString)));

        widget->setWindowTitle(tr("Message from object"));
        ui->AddWidgetToScene(widget);
        ui->ShowWidget(widget);
    }

    void ScriptDialogHandler::OnDialogClosed(int channel, const QString &answer)
    {
        // If user want't to ignore request we do not sent anything to server
        if (answer.length() > 0 && owner_->GetServerConnection()->IsConnected())
            owner_->GetServerConnection()->SendChatFromViewerPacket(std::string(answer.toUtf8()).c_str(), channel);
    }

} // end of namespace: RexLogic
