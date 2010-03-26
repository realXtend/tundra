// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiNotificationServices_h
#define incl_UiModule_UiNotificationServices_h

// QUICK TUTORIAL TO THE NOTIFICATION SERVICE
// ==========================================
// Accuire UiModule pointer to gain access to the services
// boost::shared_ptr<UiServices::UiModule> ui_module = GetFramework()->GetModuleManager()->
//     GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
//
// You can show different types of notifications through NotificationManager
// Here is how you show and hook up the notifications
//
// UiServices::MessageNotification *message = new UiServices::MessageNotification("Your message");
// 
// UiServices::InputNotification *input = new UiServices::InputNotification("Your question");
// connect(input, SIGNAL(InputRecieved(QString), SLOT(MyInputHandler(QString)));
//
// UiServices::QuestionNotification *question = new UiServices::QuestionNotification("Your question");
// connect(question, SIGNAL(QuestionAnswered(QString), SLOT(MyQuestionHandler(QString)));
//
// UiServices::ProgressController *controller = new UiServices::ProgressController();
// UiServices::ProgressNotification *progress = new UiServices::ProgressNotification("Your message", controller);
//
// ui_module->GetNotificationManager()->ShowNotification(message);
// ui_module->GetNotificationManager()->ShowNotification(input);
// ui_module->GetNotificationManager()->ShowNotification(question);
// ui_module->GetNotificationManager()->ShowNotification(progress);
//
// controller->Start();
// controller->SetValue(13);
// controller->Finish(); or controller->FailWithReason("Fail reason");

#include "Inworld/NotificationManager.h"

#include "Inworld/Notifications/NotificationBaseWidget.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/InputNotification.h"
#include "Inworld/Notifications/QuestionNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"

#endif