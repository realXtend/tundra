/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LoginScreenModule.h"
#include "LoginWidget.h"

#include "UiServiceInterface.h"
#include "InputServiceInterface.h"
#include "UiProxyWidget.h"
#include "EventManager.h"
#include "../ProtocolUtilities/NetworkEvents.h"

#include "MemoryLeakCheck.h"

std::string LoginScreenModule::type_name_static_ = "LoginScreen";

LoginScreenModule::LoginScreenModule() : ModuleInterface(type_name_static_), window_(0)
{
}

LoginScreenModule::~LoginScreenModule()
{
}

void LoginScreenModule::PreInitialize()
{
}

void LoginScreenModule::Initialize()
{
}

void LoginScreenModule::PostInitialize()
{
    input_ = framework_->Input().RegisterInputContext("LoginScreenInput", 101);
    input_->SetTakeKeyboardEventsOverQt(true);
    connect(input_.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyEvent(KeyEvent *)));

    Foundation::UiServiceInterface *ui = framework_->GetService<Foundation::UiServiceInterface>();
    if (ui)
    {
        window_ = new LoginWidget(0, QMap<QString, QString>());
        //window_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //window_->setWindowState(Qt::WindowMaximized);
        UiProxyWidget *proxy = ui->AddWidgetToScene(window_, Qt::Widget);
        proxy->setPos(0,0);
        proxy->show();

        Foundation::LoginServiceInterface *loginHandler = framework_->GetService<Foundation::LoginServiceInterface>();
        if (loginHandler)
        {
            connect(window_, SIGNAL(ConnectOpenSim(const map<QString, QString> &)), loginHandler, SLOT(ProcessLoginData(const map<QString, QString> &)));
            connect(window_, SIGNAL(ConnectRealXtend(const map<QString, QString> &)), loginHandler, SLOT(ProcessLoginData(const map<QString, QString> &)));
            
            connect(loginHandler, SIGNAL(LoginStarted()), window_, SLOT(StatusUpdate(true, const QString &)));
            connect(loginHandler, SIGNAL(LoginFailed(const QString &)), window_, SLOT(StatusUpdate(false, QString &)));
            connect(loginHandler, SIGNAL(LoginSuccessful()), window_, SLOT(test()));
        }
    }
}

void LoginScreenModule::Uninitialize()
{
    SAFE_DELETE(window_);
    input_.reset();
}

void LoginScreenModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool LoginScreenModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (framework_->GetEventManager()->QueryEventCategory("NetworkState"))
    {
        using namespace ProtocolUtilities::Events;
        switch(event_id)
        {
        case EVENT_SERVER_CONNECTED:
            // hide login screen
            break;
        case EVENT_USER_KICKED_OUT:
        case EVENT_SERVER_DISCONNECTED:
            // show login screen
            break;
        case EVENT_CONNECTION_FAILED:
            break;
        }
    }
    return false;
}

void LoginScreenModule::HandleKeyEvent(KeyEvent *key)
{
    // We only act on key presses that are not repeats.
    if (key->eventType != KeyEvent::KeyPressed || key->keyPressCount > 1)
        return;

    const QKeySequence &toggleMenu = framework_->Input().KeyBinding("LoginScreen.ToggleLoginScreen", Qt::Key_Escape);
    if (key->keyCode == toggleMenu)
    {
        // if (connected)
        //    toggle window
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(LoginScreenModule)
POCO_END_MANIFEST
