// For conditions of distribution and use, see copyright notice in license.txt

//! See UiNotificationServices.h and UiWidgetServices.h 
//! for more detailed documentation

#ifndef incl_UiModule_UiServicesFwd_h
#define incl_UiModule_UiServicesFwd_h

namespace UiServices
{
    // Main Module Class
    class UiModule;

    // Services
    class InworldSceneController;
    class NotificationManager;

    // Scene Widget Classes
    class UiProxyWidget;
    class UiWidgetProperties;

    // Notification Widget Classes
    class MessageNotification;
    class InputNotification;
    class QuestionNotification;
    class ProgressNotification;

    // Controller for ProgressNotification
    class ProgressController;
}

namespace CoreUi
{
    // Notification System Base Class
    class NotificationBaseWidget;
}

#endif