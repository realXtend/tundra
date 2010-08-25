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
#include "LoginServiceInterface.h"
#include "UiProxyWidget.h"
#include "EventManager.h"
#include "../ProtocolUtilities/NetworkEvents.h"

#include "MemoryLeakCheck.h"

/*
void StoreLoginInfo(const QMap<QString, QString> &login_info)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, classic_settings_name_);
    settings.setValue("avatar/type", classic_login_info["avatartype"]);
    settings.setValue("avatar/account", classic_login_info["account"]);
    settings.setValue("avatar/password", QByteArray(classic_login_info["password"].toStdString().c_str()).toBase64());
    settings.setValue("world/loginurl", QUrl(classic_login_info["loginurl"]));
    settings.setValue("world/startlocation", classic_login_info["startlocation"]);
}

QMap<QString, QString> GetLoginInfo() const
{
    QSettings classic_settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, classic_settings_name_);
    QMap<QString, QString> info;
    info["avatartype"] = classic_settings.value("avatar/type").toString();
    info["account"] = classic_settings.value("avatar/account").toString();
    info["password"] = QByteArray::fromBase64(classic_settings.value("avatar/password").toByteArray());
    info["loginurl"] = classic_settings.value("world/loginurl").toUrl().toString();
    info["startlocation"] = classic_settings.value("world/startlocation").toString();
    return data_map;
}
*/

std::string LoginScreenModule::type_name_static_ = "LoginScreen";

LoginScreenModule::LoginScreenModule() :
    ModuleInterface(type_name_static_),
    window_(0),
    framework_category_(0),
    network_category_(0),
    connected_(false)
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
        window_ = new LoginWidget(QMap<QString, QString>());
        connect(window_, SIGNAL(ExitClicked()), SLOT(Exit()));

        ui->AddWidgetToScene(window_, Qt::Widget);
        ui->ShowWidget(window_);

        Foundation::LoginServiceInterface *login = framework_->GetService<Foundation::LoginServiceInterface>();
        if (login)
        {
            connect(window_, SIGNAL(Connect(const QMap<QString, QString> &)),
                login, SLOT(ProcessLoginData(const QMap<QString, QString> &)));

            connect(login, SIGNAL(LoginStarted()), window_, SLOT(StartProgressBar()));

            connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(StopProgressBar()));
            connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(SetStatus(const QString &)));

            connect(login, SIGNAL(LoginSuccessful()), window_, SLOT(Connected()));
        }
    }

    framework_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");
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
    if (category_id == framework_category_ && event_id == Foundation::NETWORKING_REGISTERED)
    {
        network_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    }
    else if(category_id == network_category_)
    {
        using namespace ProtocolUtilities::Events;
        Foundation::UiServiceInterface *ui = framework_->GetService<Foundation::UiServiceInterface>();
        switch(event_id)
        {
        case EVENT_SERVER_CONNECTED:
            connected_ = true;
            if (ui && window_)
            {
                window_->SetStatus("Connected");
                ui->HideWidget(window_);
            }
            break;
        case EVENT_USER_KICKED_OUT:
        case EVENT_SERVER_DISCONNECTED:
            connected_ = false;
            if (ui && window_)
                ui->ShowWidget(window_);
            break;
        case EVENT_CONNECTION_FAILED:
            connected_ = false;
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
        Foundation::UiServiceInterface *ui = framework_->GetService<Foundation::UiServiceInterface>();
        if (connected_ && ui)
            if (!window_->isVisible())
                ui->ShowWidget(window_);
            else
                ui->HideWidget(window_);
    }
}

void LoginScreenModule::Exit()
{
    framework_->Exit();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(LoginScreenModule)
POCO_END_MANIFEST
