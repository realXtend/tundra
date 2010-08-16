// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MumbleVoipModule.h"
#include "LinkPlugin.h"
#include "ServerInfoProvider.h"
#include "ModuleManager.h"
#include "EC_OgrePlaceable.h"
#include "WorldLogicInterface.h"
#include "Entity.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"
#include "LinkPlugin.h"
#include "Provider.h"
#include "ApplicationManager.h"
#include "MumbleLibrary.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    std::string MumbleVoipModule::module_name_ = "MumbleVoip";

    MumbleVoipModule::MumbleVoipModule()
        : ModuleInterface(module_name_),
          link_plugin_(0),
          server_info_provider_(0),
          in_world_voice_provider_(0),
          time_from_last_update_ms_(0),
          use_camera_position_(false),
          use_native_mumble_client_(false)
    {
        QStringList arguments = QCoreApplication::arguments();
        foreach(QString arg, arguments)
        {
            if (arg == "--use_native_mumble_client")
                use_native_mumble_client_ = true;
        }
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(link_plugin_);
        SAFE_DELETE(server_info_provider_);
        SAFE_DELETE(in_world_voice_provider_);
    }

    void MumbleVoipModule::Load()
    {
        if (use_native_mumble_client_)
        {
            server_info_provider_ = new ServerInfoProvider(framework_);
            connect(server_info_provider_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), SLOT(StartMumbleClient(ServerInfo)));
        }
        else
        {
            in_world_voice_provider_ = new InWorldVoice::Provider(framework_);
        }

        link_plugin_ = new LinkPlugin();
    }

    void MumbleVoipModule::Unload()
    {
    }

    void MumbleVoipModule::Initialize()
    {
    }

    void MumbleVoipModule::PostInitialize()
    {
        InitializeConsoleCommands();
        
        event_category_framework_ = framework_->GetEventManager()->QueryEventCategory("Framework");
        if (event_category_framework_ == 0)
            LogError("Unable to find event category for Framework");
    }

    void MumbleVoipModule::Uninitialize()
    {
        SAFE_DELETE(link_plugin_);
        SAFE_DELETE(server_info_provider_);
        SAFE_DELETE(in_world_voice_provider_);

        if (use_native_mumble_client_)
        {
            if (ApplicationManager::StartCount() > 0)
            {
                // Hack will disconnect from server and trying to connect new one
                ApplicationManager::StartMumbleClient("mumble://user@0.0.0.0?version=1.2.2"); 
            }
        }
        else
        {
            MumbleLibrary::Stop();
        }
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
        if (link_plugin_ && link_plugin_->IsRunning())
            UpdateLinkPlugin(frametime);
        
        if (in_world_voice_provider_)
            in_world_voice_provider_->Update(frametime);
    }

    bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (in_world_voice_provider_)
            in_world_voice_provider_->HandleEvent(category_id, event_id, data);

        if (server_info_provider_)
            server_info_provider_->HandleEvent(category_id, event_id, data);

        return false;
    }

    void MumbleVoipModule::UpdateLinkPlugin(f64 frametime)
    {
        if (!link_plugin_)
            return;

        time_from_last_update_ms_ += 1000*frametime;
        if (time_from_last_update_ms_ < LINK_PLUGIN_UPDATE_INTERVAL_MS_)
            return;
        time_from_last_update_ms_ = 0;

        Vector3df top_vector = Vector3df::UNIT_Z, position, direction;
        if (GetAvatarPosition(position, direction))
            link_plugin_->SetAvatarPosition(position, direction, top_vector);

        if (use_camera_position_)
            if (GetCameraPosition(position, direction))
                link_plugin_->SetCameraPosition(position, direction, top_vector);
        else
            if (GetAvatarPosition(position, direction))
                link_plugin_->SetCameraPosition(position, direction, top_vector);

        link_plugin_->SendData();
    }

    bool MumbleVoipModule::GetAvatarPosition(Vector3df& position, Vector3df& direction)
    {
        using namespace Foundation;
        boost::shared_ptr<WorldLogicInterface> worldLogic = framework_->GetServiceManager()->GetService<WorldLogicInterface>(Service::ST_WorldLogic).lock();
        if (!worldLogic)
            return false;

        Scene::EntityPtr avatar = worldLogic->GetUserAvatarEntity();
        if (!avatar)
            return false;

        boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> ogre_placeable = avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
        if (!ogre_placeable)
            return false;

        Quaternion q = ogre_placeable->GetOrientation();
        position = ogre_placeable->GetPosition(); 
        direction = q*Vector3df::UNIT_X;
        return true;
    }

    bool MumbleVoipModule::GetCameraPosition(Vector3df& position, Vector3df& direction)
    {
        using namespace Foundation;
        boost::shared_ptr<WorldLogicInterface> worldLogic = framework_->GetServiceManager()->GetService<WorldLogicInterface>(Service::ST_WorldLogic).lock();
        if (!worldLogic)
            return false;

        Scene::EntityPtr camera = worldLogic->GetCameraEntity();
        if (!camera)
            return false;

        boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> ogre_placeable = camera->GetComponent<OgreRenderer::EC_OgrePlaceable>();
        if (!ogre_placeable)
            return false;

        Quaternion q = ogre_placeable->GetOrientation();
        position = ogre_placeable->GetPosition(); 
        direction = q*Vector3df::UNIT_X;
        return true;
    }

    void MumbleVoipModule::InitializeConsoleCommands()
    {
        RegisterConsoleCommand(Console::CreateCommand("mumble link", "Start Mumble link plugin: 'mumble link(user_id, context_id)'",
            Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleLink)));
        RegisterConsoleCommand(Console::CreateCommand("mumble unlink", "Stop Mumble link plugin: 'mumble unlink'",
            Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleUnlink)));
        RegisterConsoleCommand(Console::CreateCommand("mumble start", "Start Mumble client application: 'mumble start(server_url)'",
            Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleStart)));
        RegisterConsoleCommand(Console::CreateCommand("mumble stats", "Show mumble statistics", Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleStats)));

        //RegisterConsoleCommand(Console::CreateCommand("mumble enable vad", "Enable voice activity detector",
        //    Console::Bind(this, &MumbleVoipModule::OnConsoleEnableVoiceActivityDetector)));
        //RegisterConsoleCommand(Console::CreateCommand("mumble disable vad", "Disable voice activity detector",
        //    Console::Bind(this, &MumbleVoipModule::OnConsoleDisableVoiceActivityDetector)));
    }

    Console::CommandResult MumbleVoipModule::OnConsoleMumbleLink(const StringVector &params)
    {
        if (!link_plugin_)
            return Console::ResultFailure("Link plugin is not initialized.");
        if (params.size() != 2)
        {
            return Console::ResultFailure("Wrong number of arguments: usage 'mumble link(id, context)'");
        }
        QString id = params[0].c_str();
        QString context = params[1].c_str();
        
        link_plugin_->SetUserIdentity(id);
        link_plugin_->SetContextId(context);
        link_plugin_->SetApplicationName("Naali viewer");
        link_plugin_->SetApplicationDescription("Naali viewer by realXtend project");
        link_plugin_->Start();

        if (!link_plugin_->IsRunning())
        {
            QString error_message = "Link plugin connection cannot be established. ";
            error_message.append(link_plugin_->GetReason());
            return Console::ResultFailure(error_message.toStdString());
        }

        QString message = QString("Mumbe link plugin started: id=%1 context=%2").arg(id).arg(context);
        return Console::ResultSuccess(message.toStdString());
    }

    Console::CommandResult MumbleVoipModule::OnConsoleMumbleUnlink(const StringVector &params)
    {
        if (!link_plugin_)
            return Console::ResultFailure("Link plugin is not initialized.");
        if (params.size() != 0)
        {
            return Console::ResultFailure("Wrong number of arguments: usage 'mumble unlink'");
        }

        if (!link_plugin_->IsRunning())
        {
            return Console::ResultFailure("Mumbe link plugin was not running.");
        }

        link_plugin_->Stop();
        return Console::ResultSuccess("Mumbe link plugin stopped.");
    }

    Console::CommandResult MumbleVoipModule::OnConsoleMumbleStart(const StringVector &params)
    {
        if (params.size() != 1)
        {
            return Console::ResultFailure("Wrong number of arguments: usage 'mumble start(server_url)'");
        }
        QString server_url = params[0].c_str();

        try
        {
            ApplicationManager::StartMumbleClient(server_url);
            return Console::ResultSuccess("Mumbe client started.");
        }
        catch(std::exception &e)
        {
            QString error_message = QString("Cannot start Mumble client: %1").arg(QString(e.what()));
            return Console::ResultFailure(error_message.toStdString());        
        }
    }

    void MumbleVoipModule::StartMumbleClient(ServerInfo info)
    {
        QUrl murmur_url(QString("mumble://%1/%2").arg(info.server).arg(info.channel)); // setScheme method does not add '//' between scheme and host.
        murmur_url.setUserName(info.user_name);
        murmur_url.setPassword(info.password);
        murmur_url.setQueryItems(QList<QPair<QString,QString> >() << QPair<QString,QString>("version", info.version));

        try
        {
            LogInfo("Starting mumble client.");
            ApplicationManager::StartMumbleClient(murmur_url.toString());

            // it takes some time for a mumble client to setup shared memory for link plugins
            // so we have to wait some time before we can start our link plugin.
            avatar_id_for_link_plugin_ = info.avatar_id;
            context_id_for_link_plugin_ = info.context_id;
            QTimer::singleShot(2000, this, SLOT(StartLinkPlugin()));
        }
        catch(std::exception &e)
        {
            QString messge = QString("Cannot start Mumble client: %1").arg(e.what());
            LogError(messge.toStdString());
            return;
        }
    }

    void MumbleVoipModule::StartLinkPlugin()
    {
        if (!link_plugin_)
            return;

        link_plugin_->SetUserIdentity(avatar_id_for_link_plugin_);
        link_plugin_->SetContextId(context_id_for_link_plugin_);
        link_plugin_->SetApplicationName("Naali viewer");
        link_plugin_->SetApplicationDescription("Naali viewer by realXtend project");
        link_plugin_->Start();

        if (link_plugin_->IsRunning())
        {
            QString message = QString("Mumbe link plugin started: id '%1' context '%2'").arg(avatar_id_for_link_plugin_).arg(context_id_for_link_plugin_);
            LogInfo(message.toStdString());
        }
        else
        {
            QString error_message = QString("Link plugin connection cannot be established. %1 ").arg(link_plugin_->GetReason());
            LogError(error_message.toStdString());
        }
    }

    Console::CommandResult MumbleVoipModule::OnConsoleMumbleStats(const StringVector &params)
    {
        
        if (in_world_voice_provider_)
        {
            QList<QString> stats = in_world_voice_provider_->Statistics();
            foreach(QString line, stats)
            {
                LogInfo(line.toStdString());
            }
        }
        QString message = QString("");
        return Console::ResultSuccess(message.toStdString());
    }

//    Console::CommandResult MumbleVoipModule::OnConsoleEnableVoiceActivityDetector(const StringVector &params)
//    {
////        connection_manager_->EnableVAD(true);
//        QString message = QString("Voice activity detector enabled.");
//        return Console::ResultSuccess(message.toStdString());
//    }

  //  Console::CommandResult MumbleVoipModule::OnConsoleDisableVoiceActivityDetector(const StringVector &params)
  //  {
  ////      connection_manager_->EnableVAD(false);
  //      QString message = QString("Voice activity detector disabled.");
  //      return Console::ResultSuccess(message.toStdString());
  //  }

} // end of namespace: MumbleVoip

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace MumbleVoip;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(MumbleVoipModule)
POCO_END_MANIFEST
