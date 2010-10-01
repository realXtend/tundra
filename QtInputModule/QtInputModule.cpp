// For conditions of distribution and use, see copyright notice in license.txt

#include "ServiceManager.h"
#include "InputEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "QtInputModule.h"
#include "KeyBindingsConfigWindow.h"
#include "ConsoleCommandServiceInterface.h"

#include "UiServiceInterface.h"
#include "UiSettingsServiceInterface.h"
#include "UiProxyWidget.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QApplication>
#include <QPushButton>
#include <QGraphicsProxyWidget>

#include <boost/make_shared.hpp>

QtInputModule::QtInputModule() :
    IModule("QtInput"),
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
    Foundation::UiServiceInterface *ui = framework_->GetService<Foundation::UiServiceInterface>();
    configWindow = new KeyBindingsConfigWindow(framework_);

    if (!ui)
        return;

    ui->AddSettingsWidget(configWindow, "Controls");

    Foundation::UiSettingsServiceInterface *sett_ui = framework_->GetService<Foundation::UiSettingsServiceInterface>();
    if(!sett_ui)
        return;

    QObject *settings_widget_ = sett_ui->GetMainSettingsWidget();
   
    connect(settings_widget_, SIGNAL(SaveSettingsClicked()), configWindow, SLOT(ButtonOK()));
    connect(settings_widget_, SIGNAL(CancelClicked()), configWindow, SLOT(ButtonCancel()));
}

void QtInputModule::Update(f64 frametime)
{
    inputService->Update(frametime);

    RESETPROFILER;
}

static std::string moduleName = "QtInput";
const std::string &QtInputModule::NameStatic()
{
    return moduleName;
}

void QtInputModule::ShowBindingsWindow()
{
    UiServicePtr ui = framework_->GetService<UiServiceInterface>(Foundation::Service::ST_Gui).lock();
    if (!ui)
        return;

    if (configWindow)
    {
        ui->BringWidgetToFront(configWindow);
        return;
    }

    configWindow = new KeyBindingsConfigWindow(framework_);

    UiProxyWidget *proxy = ui->AddWidgetToScene(configWindow);

    QPushButton *btn = configWindow->findChild<QPushButton*>("pushButtonCancel");
    QObject::connect(btn, SIGNAL(pressed()), this, SLOT(BindingsWindowClosed()));
    btn = configWindow->findChild<QPushButton*>("pushButtonOK");
    QObject::connect(btn, SIGNAL(pressed()), this, SLOT(BindingsWindowClosed()));

    proxy->resize(350, 230);
    proxy->show();
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
