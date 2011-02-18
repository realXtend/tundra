/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EtherModule.h"
#include "EtherLogic.h"

#include "UiServiceInterface.h"
#include "Input.h"
#include "LoginServiceInterface.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"
#include "EventManager.h"

//#ifdef ENABLE_TAIGA_LOGIN
#include "../ProtocolUtilities/NetworkEvents.h"
//#endif

#include "../TundraLogicModule/TundraEvents.h"

#include "MemoryLeakCheck.h"

namespace Ether
{

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("Ether")

std::string EtherModule::type_name_static_ = "Ether";

EtherModule::EtherModule() :
    IModule(type_name_static_),
    ether_logic_(0),
    framework_category_(0),
    network_category_(0),
    connected_(false)
{
}

EtherModule::~EtherModule()
{
}

void EtherModule::PreInitialize()
{
}

void EtherModule::Initialize()
{
}

void EtherModule::PostInitialize()
{
    input_ = framework_->GetInput()->RegisterInputContext("EtherInput", 101);
    input_->SetTakeKeyboardEventsOverQt(true);
    connect(input_.get(), SIGNAL(KeyPressed(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));

    UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    if (ui)
    {
		// Start ether logic and register to scene service
		ether_logic_ = new Ether::Logic::EtherLogic(GetFramework());
		ui->RegisterScene("Ether", ether_logic_->GetScene());
		ether_logic_->Start();
		// Switch ether scene active on startup
		ui->SwitchToScene("Ether");
		// Connect the switch signal to needed places
		connect(ui, SIGNAL(SceneChanged(const QString&, const QString&)), 
			ether_logic_->GetQObjSceneController(), SLOT(UiServiceSceneChanged(const QString&, const QString&)));
		LogDebug("Ether Logic STARTED");


        //window_ = new LoginWidget(framework_);
        //connect(window_, SIGNAL(ExitClicked()), SLOT(Exit()));

        //ui->AddWidgetToScene(window_, Qt::Widget);
        //ui->ShowWidget(window_);

        //Foundation::LoginServiceInterface *login = framework_->GetService<Foundation::LoginServiceInterface>();
        //if (login)
        //{
        //    connect(window_, SIGNAL(Connect(const QMap<QString, QString> &)),
        //        login, SLOT(ProcessLoginData(const QMap<QString, QString> &)));

        //    // Connect also to Tundra login
        //    connect(window_, SIGNAL(Connect(const QMap<QString, QString> &)),
        //        this, SLOT(ProcessTundraLogin(const QMap<QString, QString> &)));
        //    
        //    connect(login, SIGNAL(LoginStarted()), window_, SLOT(StartProgressBar()));

        //    connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(StopProgressBar()));
        //    connect(login, SIGNAL(LoginFailed(const QString &)), window_, SLOT(SetStatus(const QString &)));

        //    connect(login, SIGNAL(LoginSuccessful()), window_, SLOT(Connected()));
        //}
    }

    framework_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    tundra_category_ = framework_->GetEventManager()->QueryEventCategory("Tundra");
}

void EtherModule::Uninitialize()
{
	SAFE_DELETE(ether_logic_);
    input_.reset();
}

void EtherModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool EtherModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    
    if (category_id == framework_category_ && event_id == Foundation::NETWORKING_REGISTERED)
    {
        network_category_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    }
//#ifdef ENABLE_TAIGA_LOGIN
    else if(category_id == network_category_)
    {
        using namespace ProtocolUtilities::Events;
        switch(event_id)
        {
        case EVENT_SERVER_CONNECTED:
            connected_ = true;
			ether_logic_->GetSceneController()->ShowStatusInformation("Connected, loading world content...", 60000);
            break;
        case EVENT_USER_KICKED_OUT:
        case EVENT_SERVER_DISCONNECTED:
            connected_ = false;
			ether_logic_->SetConnectionState(Disconnected);
            //if (ui && window_)
            //    ui->ShowWidget(window_);
            break;
        case EVENT_CONNECTION_FAILED:
            connected_ = false;
			ether_logic_->SetConnectionState(Failed);
            break;
        }
    }
//#endif
    else if (category_id == tundra_category_)
    {
        switch(event_id)
        {
        case TundraLogic::Events::EVENT_TUNDRA_CONNECTED:
            connected_ = true;
            //if (ui && window_)
            //{
            //    window_->Connected();
            //    ui->HideWidget(window_);
            //}
            break;
        case TundraLogic::Events::EVENT_TUNDRA_DISCONNECTED:
            connected_ = false;
            //if (ui && window_)
            //    ui->ShowWidget(window_);
            break;
        }
    }
    
    return false;
}

void EtherModule::HandleKeyEvent(KeyEvent *key)
{
    // We only act on key presses that are not repeats.
    if (key->eventType != KeyEvent::KeyPressed || key->keyPressCount > 1)
        return;

    const QKeySequence &toggleMenu = framework_->GetInput()->KeyBinding("Ether.ToggleEther", Qt::Key_Escape);
    if (key->keyCode == toggleMenu)
    {
        //UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        //if (connected_ && ui)
        //    if (!window_->isVisible())
        //        ui->ShowWidget(window_);
        //    else
        //        ui->HideWidget(window_);
    }
}

void EtherModule::ProcessTundraLogin(const QMap<QString, QString> &data)
{
    if (data["AvatarType"] == "Tundra")
    {
        std::string worldAddress = data["WorldAddress"].toStdString();
        unsigned short port = 0; // Use default if not specified
        
        size_t pos = worldAddress.find(':');
        if (pos != std::string::npos)
        {
            try
            {
                port = ParseString<int>(worldAddress.substr(pos + 1));
            }
            catch (...) {}
            worldAddress = worldAddress.substr(0, pos);
        }
        
        TundraLogic::Events::TundraLoginEventData logindata;
        logindata.address_ = worldAddress;
        logindata.port_ = port;
        logindata.username_ = data["Username"].toStdString();
        logindata.password_ = data["Password"].toStdString();
        logindata.protocol_ = data["Protocol"].toStdString();
        LogInfo("Attempting Tundra connection to " + worldAddress + " as " + logindata.username_);
        framework_->GetEventManager()->SendEvent(tundra_category_, TundraLogic::Events::EVENT_TUNDRA_LOGIN, &logindata);
    }
}

void EtherModule::Exit()
{
    framework_->Exit();
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Ether;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(EtherModule)
POCO_END_MANIFEST
