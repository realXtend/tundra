/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NetworkStateEventHandler.h
 *  @brief  Handles events related to changes in server connectivity in a RexLogic-specific way.
 */

#include "StableHeaders.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "RexLogicModule.h"
#include "Communications/InWorldChat/Provider.h"

#include "NetworkEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "WorldStream.h"

#ifndef UISERVICE_TEST
#include "UiModule.h"
#include "Inworld/NotificationManager.h"
#include "Inworld/Notifications/MessageNotification.h"
#endif
//#include <QMessageBox>

namespace RexLogic
{

NetworkStateEventHandler::NetworkStateEventHandler(RexLogicModule *owner) : owner_(owner)
{
}

NetworkStateEventHandler::~NetworkStateEventHandler()
{
}

bool NetworkStateEventHandler::HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    switch(event_id)
    {
    case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
    {
        // Set auth data to the current world stream
        ProtocolUtilities::AuthenticationEventData *inbound_event_data = checked_static_cast<ProtocolUtilities::AuthenticationEventData*>(data);
        if (inbound_event_data)
            owner_->GetServerConnection()->SetAuthenticationType(inbound_event_data->type);
            
        // The client has connected to the server. Create a new scene for that.
        owner_->CreateNewActiveScene("World");
        // Send WorldStream as internal event
        event_category_id_t framework_category_id = owner_->GetFramework()->GetEventManager()->QueryEventCategory("Framework");
        ProtocolUtilities::WorldStreamReadyEvent event_data(owner_->GetServerConnection());
        owner_->GetFramework()->GetEventManager()->SendEvent(framework_category_id, Foundation::WORLD_STREAM_READY, &event_data);
        break;
    }
    case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
    {
        // Reset the authentication type
        owner_->GetServerConnection()->SetAuthenticationType(ProtocolUtilities::AT_Unknown);
        
        // Might be user quitting or server dropping connection.
        // This event occurs when OpenSimProtocolModule has already closed connection. 
        // Make sure the rexlogic also thinks connection is closed.
        if (owner_->GetServerConnection()->IsConnected())
            owner_->GetServerConnection()->ForceServerDisconnect();
        if (owner_->GetFramework()->HasScene("World"))
            owner_->DeleteScene("World");
        break;
    }
    case ProtocolUtilities::Events::EVENT_USER_CONNECTED:
    {
        /// Show notification about new user
        ProtocolUtilities::UserConnectivityEvent *event_data = checked_static_cast<ProtocolUtilities::UserConnectivityEvent *>(data);
        assert(event_data);
        if (!event_data)
            return false;
#ifndef UISERVICE_TEST
        UiServices::UiModule *ui_module = owner_->GetFramework()->GetModule<UiServices::UiModule>();
        if (ui_module)
            ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification(
                QString("%1 joined the world").arg(event_data->fullName.c_str())));
#endif
        break;
    }
    case ProtocolUtilities::Events::EVENT_USER_DISCONNECTED:
    {
        ProtocolUtilities::UserConnectivityEvent *event_data = checked_static_cast<ProtocolUtilities::UserConnectivityEvent *>(data);
        assert(event_data);
        if (!event_data)
            return false;
#ifndef UISERVICE_TEST
        UiServices::UiModule *ui_module = owner_->GetFramework()->GetModule<UiServices::UiModule>();
        if (ui_module)
            ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification(
                QString("%1 logged out").arg(event_data->fullName.c_str())));
#endif
        break;
    }
    case ProtocolUtilities::Events::EVENT_USER_KICKED_OUT:
    {
        owner_->LogoutAndDeleteWorld();
        // Show dialog
        /* now uimodule shows this using some non-blocking system
           - this version blocks the Naali mainloop which not always nice (when running as a server)
        QWidget* mainwindow = 0;
        boost::shared_ptr<OgreRenderer::Renderer> renderer =
            owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
            mainwindow = owner_->GetFramework()->GetMainWindow();
        QMessageBox msgBox(QMessageBox::Warning, QApplication::translate("RexLogic", "Kicked Out"), 
            QApplication::translate("RexLogic", "You were kicked out from the server."), QMessageBox::Ok, mainwindow);
        msgBox.exec();*/
        break;
    }
    default:
        break;
    }

    return false;
}

}
