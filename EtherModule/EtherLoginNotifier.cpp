// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EtherLoginNotifier.h"
#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"

#include "EtherModule.h"
//#include "Inworld/InworldSceneController.h"
//#include "Inworld/ControlPanelManager.h"
//#include "Inworld/ControlPanel/TeleportWidget.h"
//#include "UiNotificationServices.h"

#include "ModuleManager.h"
#include "LoginServiceInterface.h"
#include "UiServiceInterface.h"
#include "TundraLogicModule.h"
#include "Client.h"

#include <QWebFrame>
#include <QTimer>

#include "MemoryLeakCheck.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EtherLogin");

namespace Ether
{
    namespace Logic
    {
        EtherLoginNotifier::EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller, Foundation::Framework *framework) :
            QObject(parent),
            scene_controller_(scene_controller),
            framework_(framework),
            teleporting_(false)
        {
            // Get login handler and connect signals
            Foundation::LoginServiceInterface *handler = framework_->GetService<Foundation::LoginServiceInterface>();
            if (handler)
            {
                connect(this, SIGNAL(StartLogin(const QMap<QString, QString> &)), handler, SLOT(ProcessLoginData(const QMap<QString, QString> &)));
                connect(this, SIGNAL(StartLogin(QWebFrame *)), handler, SLOT(ProcessLoginData(QWebFrame *)));
                connect(this, SIGNAL(StartLogin(const QString &)), handler, SLOT(ProcessLoginData(const QString &)));

                connect(this, SIGNAL(Disconnect()), handler, SLOT(Logout()));
                connect(this, SIGNAL(Quit()), handler, SLOT(Quit()));

                connect(handler, SIGNAL(LoginStarted()), SLOT(EmitLoginStarted()));
                connect(handler, SIGNAL(LoginFailed(const QString &)), SLOT(EmitLoginFailed(const QString &)));
                connect(handler, SIGNAL(LoginSuccessful()), SLOT(EmitLoginSuccessful()));
            }
            else
                LogError("Could not retrieve login service.");

			// $ BEGIN_MOD $
			connect (framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient().get(), SIGNAL(Connected()), SLOT(EmitLoginSuccessful()));
			connect (this, SIGNAL(Disconnect()), framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient().get(), SLOT(Logout()));
			//connect (framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient().get(), SIGNAL(Disconnected()), owner_ SLOT(ControlsWidgetHandler("")));
			// $ END_MOD $

			//@todo: teleport
            //UiServices::UiModule *ui_module =  framework_->GetModule<UiServices::UiModule>();
            //if (ui_module)
            //    connect(ui_module->GetInworldSceneController()->GetControlPanelManager()->GetTeleportWidget(), SIGNAL(StartTeleport(QString)), SLOT(Teleport(QString)));
        }

        void EtherLoginNotifier::ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards)
        {
            QMap<QString, QString> info_map;
            switch (data_cards.first->avatarType())
            {
                case AvatarTypes::OpenSim:
                {
                    Data::OpenSimAvatar *oa = checked_static_cast<Data::OpenSimAvatar*>(data_cards.first);
                    assert(oa);
                    info_map["Username"] = oa->userName();
                    info_map["Password"] = oa->password();
                    info_map["AvatarType"] = "OpenSim";
                    break;
                }
                case AvatarTypes::RealXtend:
                {
                    Data::RealXtendAvatar *ra = checked_static_cast<Data::RealXtendAvatar*>(data_cards.first);
                    assert(ra);
                    info_map["Username"] = ra->account();
                    info_map["Password"] = ra->password();
                    info_map["AuthenticationAddress"] = ra->authUrl().toString();
                    info_map["AvatarType"] = "RealXtend";
                    break;
                }
            }
			switch (data_cards.second->worldType())
            {
                case WorldTypes::OpenSim:
                {
                    Data::WorldInfo *ow = checked_static_cast<Data::WorldInfo*>(data_cards.second);
                    assert(ow);
                    info_map["WorldAddress"] = ow->loginUrl().toString();
                    info_map["StartLocation"] = ow->startLocation();

                    emit StartLogin(info_map);
                    break;
                }
				case WorldTypes::Tundra:
				{
					Data::WorldInfo *ow = checked_static_cast<Data::WorldInfo*>(data_cards.second);
                    assert(ow);
                    info_map["WorldAddress"] = ow->loginUrl().toString();
                    info_map["StartLocation"] = ow->startLocation();
					info_map["WorldHost"] = ow->loginUrl().host();
					info_map["WorldPort"] = QString::number( ow->loginUrl().port() );
					if (ow->gridInfo().contains("Protocol"))
						info_map["Protocol"] = ow->gridInfo()["Protocol"].toString();
					last_info_map_ = info_map;
					emit StartTundraLogin(info_map);
                    break;
				}
            }
        }

        void EtherLoginNotifier::EmitLogin(const QMap<QString, QString> &info_map)
        {
            emit StartLogin(info_map);
        }

        void EtherLoginNotifier::EmitLogin(QWebFrame *web_frame)
        {
            emit StartLogin(web_frame);
        }

        void EtherLoginNotifier::EmitLogin(const QString &url)
        {
            emit StartLogin(url);
        }

        void EtherLoginNotifier::EmitDisconnectRequest()
        {
            emit Disconnect();
        }

        void EtherLoginNotifier::EmitLoginStarted()
        {
            emit LoginStarted();
        }

        void EtherLoginNotifier::EmitLoginFailed(const QString &message)
        {
            emit LoginFailed(message);
        }

        void EtherLoginNotifier::EmitLoginSuccessful()
        {
            emit LoginSuccessful();
        }

        void EtherLoginNotifier::ExitApplication()
        {
            emit Quit();
        }

        void EtherLoginNotifier::ScriptTeleportAnswer(QString answer, QString region_name)
        {
            answer = answer.toLower();
            if (answer == "yes")
            {
                QTimer::singleShot(1000, this, SLOT(ScriptTeleport()));
                region_name_ = region_name;
            }
            else if (answer == "no")
            {
                teleporting_ = false;
                region_name_ = "";
            }
        }

        void EtherLoginNotifier::ScriptTeleport()
        {
            if (!region_name_.isEmpty())
                Teleport(region_name_);
            teleporting_ = false;
        }

        void EtherLoginNotifier::Teleport(QString start_location)
        {
            if (start_location.isEmpty())
                return;

            last_info_map_["StartLocation"] = start_location;
            QString avatar_type = last_info_map_["AvatarType"].toLower();

            if (avatar_type == "opensim")
                emit StartLogin(last_info_map_);
            else if (avatar_type == "realxtend")
                emit StartLogin(last_info_map_);
            else
            {
                LogError("Webauth avatars can't teleport yet.");
				UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
				//@todo enable when NotificationBaseWidget is in a public interface
				//if (ui)
				//	ui->ShowNotification(new CoreUi::NotificationBaseWidget(5000, "Webauth avatars cannot teleport yet, sorry."));
                //boost::shared_ptr<UiServices::UiModule> ui_module =  framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
                //if (ui_module)
                //    ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification("Webauth avatars cannot teleport yet, sorry."));
            }
        }
    }
}
