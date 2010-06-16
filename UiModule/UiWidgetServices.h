// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiWidgetServices_h
#define incl_UiModule_UiWidgetServices_h

// QUICK TUTORIAL TO THE WIDGET SERVICES
// ==========================================
// Acquire UiModule pointer to gain access to the services
// boost::shared_ptr<UiServices::UiModule> ui_module = GetFramework()->GetModuleManager()->
//     GetModule<UiServices::UiModule>().lock();
//
// You can add any QWidget to the inworld scene from your module. First you need to define
// some properties. You have two type options ModuleWidget or SceneWidget. The difference is that
// ModuleWidget will be shown in the toolbar and SceneWidget not. Note that you cant bring back
// a SceneWidget after you hide/close the window from the UI, you will have to control this yourself.
// For ModuleWidgets this is what the toolbar is for.
//
// UiServices::UiWidgetProperties properties("My Widget Name", UiServices::ModuleWidget);
// UiServices::UiProxyWidget *proxy_widget = new UiServices::UiProxyWidget(my_q_widget, properties);
//
// Now you can add the UiProxyWidget to the ui by calling the InworldSceneController
// ui_module->GetInworldSceneController()->AddProxyWidget(proxy_widget);
//
// Alternatively you can call AddWidgetToScene() and get a proxy returned to you
// proxy_widget = ui_module->GetInworldSceneController()->AddWidgetToScene(my_q_widget, properties);

#include "Inworld/InworldSceneController.h"

#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/View/UiProxyWidget.h"

#endif