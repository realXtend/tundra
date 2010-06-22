// For conditions of distribution and use, see copyright notice in license.txt

#include "ServiceManager.h"
#include "InputEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "QtInputModule.h"
#include "KeyBindingsConfigWindow.h"
#include "ConsoleCommandServiceInterface.h"
#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QApplication>
#include <QPushButton>

#include <boost/make_shared.hpp>

QtInputModule::QtInputModule() 
:ModuleInterface("QtInputModule"),
configWindow(0)
{
}

QtInputModule::~QtInputModule()
{
}

void QtInputModule::Initialize()
{
    inputService = boost::make_shared<QtInputService>(framework_);
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Input, inputService);
}

void QtInputModule::PostInitialize()
{
    RegisterConsoleCommand(Console::CreateCommand("bindings", 
        "Shows the bindings configuration window.",
        Console::Bind(this, &QtInputModule::ShowBindingsWindowConsole)));
}

void QtInputModule::Update(f64 frametime)
{
    inputService->Update(frametime);

    RESETPROFILER;
}

static std::string moduleName = "QtInputModule";
const std::string &QtInputModule::NameStatic()
{
    return moduleName;
}

void QtInputModule::ShowBindingsWindow()
{
    UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
    if (!ui_module)
        return;

    configWindow = new KeyBindingsConfigWindow(framework_);
    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(configWindow,
        UiServices::UiWidgetProperties("Actions", UiServices::SceneWidget));
    ui_module->GetInworldSceneController()->ShowProxyForWidget(configWindow);

    QPushButton *btn = configWindow->findChild<QPushButton*>("pushButtonCancel");
    QObject::connect(btn, SIGNAL(pressed()), this, SLOT(BindingsWindowClosed()));
    btn = configWindow->findChild<QPushButton*>("pushButtonOK");
    QObject::connect(btn, SIGNAL(pressed()), this, SLOT(BindingsWindowClosed()));

    proxy->resize(350, 230);
    proxy->show();
    QObject::connect(proxy, SIGNAL(Closed()), configWindow, SLOT(deleteLater()));
}

void QtInputModule::BindingsWindowClosed()
{
    configWindow = 0;
}

Console::CommandResult QtInputModule::ShowBindingsWindowConsole(const StringVector &params)
{
    ShowBindingsWindow();
    return Console::ResultSuccess();
}
