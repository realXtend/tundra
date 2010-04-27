// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MumbleVoipModule.h"
#include "RexLogicModule.h"
#include "ModuleManager.h"
#include "Avatar/Avatar.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"

#include "LinkPlugin.h"
#include "ServerObserver.h"
#include "ConnectionManager.h"
#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    std::string MumbleVoipModule::module_name_ = "MumbleVoipModule";

    MumbleVoipModule::MumbleVoipModule()
        : ModuleInterfaceImpl(module_name_),
          link_plugin_(0),
          time_from_last_update_ms_(0),
          server_observer_(0),
          connection_manager_(0),
          use_camera_position_(false)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
    }

    void MumbleVoipModule::Load()
    {
        connection_manager_ = new ConnectionManager();
        link_plugin_ = new LinkPlugin();
        server_observer_ = new ServerObserver(framework_);
        connect(server_observer_, SIGNAL(MumbleServerInfoReceived(ServerInfo)), this, SLOT(OnMumbleServerInfoReceived(ServerInfo)) );
    }

    void MumbleVoipModule::Unload()
    {
        SAFE_DELETE(connection_manager_);
        SAFE_DELETE(link_plugin_);
        SAFE_DELETE(server_observer_);
    }

    void MumbleVoipModule::Initialize() 
    {
        InitializeConsoleCommands();
    }

    void MumbleVoipModule::PostInitialize()
    {
    }

    void MumbleVoipModule::Uninitialize()
    {
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
        if (link_plugin_ && link_plugin_->IsRunning())
            UpdateLinkPlugin(frametime);
    }

    bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (server_observer_)
            server_observer_->HandleEvent(category_id, event_id, data);

        return false;
    }

    void MumbleVoipModule::UpdateLinkPlugin(f64 frametime)
    {
        if (!link_plugin_)
            return;

        time_from_last_update_ms_ += 1000*frametime;
        if (time_from_last_update_ms_ < UPDATE_TIME_MS_)
            return;
        time_from_last_update_ms_ = 0;
        ///\todo Remove RexLogicModule dependency!
        /// Iterate scene entities and get EC_OpenSimPresence. If EC_OpenSimPresence exists and it agentId
        /// matches with worlstream->GetInfo().agentId it's our user's entity.
        RexLogic::RexLogicModule *rex_logic_module = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
        if (!rex_logic_module)
            return;

        RexLogic::AvatarPtr avatar = rex_logic_module->GetAvatarHandler();
        if (avatar)
        {
            Scene::EntityPtr entity = avatar->GetUserAvatar();
            if (!entity)
                return;

            const Foundation::ComponentInterfacePtr &placeable_component = entity->GetComponent("EC_OgrePlaceable");
            if (placeable_component)
            {
                Vector3df top_vector = Vector3df::UNIT_Z;
                OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
                Quaternion q = ogre_placeable->GetOrientation();
                Vector3df position_vector = ogre_placeable->GetPosition(); 
                Vector3df front_vector = q*Vector3df::UNIT_X;

                link_plugin_->SetAvatarPosition(position_vector, front_vector, top_vector);
                if (!use_camera_position_)
                    link_plugin_->SetCameraPosition(position_vector, front_vector, top_vector);
            }

            ///\todo Remove RexLogicModule dependency!
            /// Iterate scene entities and get EC_OgreCamera. If EC_OgreCamera exists and it's active
            /// use that entity.
            Scene::EntityPtr camera = rex_logic_module->GetCameraEntity().lock();
            if (camera)
            {
                const Foundation::ComponentInterfacePtr &placeable_component = camera->GetComponent("EC_OgrePlaceable");
                if (placeable_component)
                {
                    Vector3df top_vector = Vector3df::UNIT_Z;

                    OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
                    Quaternion q = ogre_placeable->GetOrientation();

                    Vector3df position_vector = ogre_placeable->GetPosition(); 
                    Vector3df front_vector = q*Vector3df::UNIT_X;
                    if (use_camera_position_)
                        link_plugin_->SetCameraPosition(position_vector, front_vector, top_vector);
                }
            }

            link_plugin_->SendData();
        }
    }

    void MumbleVoipModule::InitializeConsoleCommands()
    {
        boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if (console_service)
        {
            console_service->RegisterCommand(Console::CreateCommand("mumble link", "Start Mumble link plugin: 'mumble link(user_id, context_id)'", Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleLink)));
            console_service->RegisterCommand(Console::CreateCommand("mumble unlink", "Stop Mumble link plugin: 'mumble unlink'", Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleUnlink)));
            console_service->RegisterCommand(Console::CreateCommand("mumble start", "Start Mumble client application: 'mumble start(server_url)'", Console::Bind(this, &MumbleVoipModule::OnConsoleMumbleStart)));
        }
    }
    
    Console::CommandResult MumbleVoipModule::OnConsoleMumbleLink(const StringVector &params)
    {
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
            ConnectionManager::StartMumbleClient(server_url);
            return Console::ResultSuccess("Mumbe client started.");
        }
        catch(std::exception &e)
        {
            QString error_message = QString("Cannot start Mumble client: %1").arg(QString(e.what()));
            return Console::ResultFailure(error_message.toStdString());        
        }
    }

    void MumbleVoipModule::OnMumbleServerInfoReceived(ServerInfo info)
    {
        QUrl murmur_url(QString("mumble://%1/%2").arg(info.server).arg(info.channel)); // setScheme method does not add '//' between scheme and host.
        murmur_url.setUserName(info.user_name);
        murmur_url.setPassword(info.password);
        murmur_url.setQueryItems(QList<QPair<QString,QString> >() << QPair<QString,QString>("version", info.version));

        LogInfo("Starting mumble client.");
        try
        {
            ConnectionManager::StartMumbleClient(murmur_url.toString());

            // it takes some time for a mumble client to setup shared memory for link plugins
            // so we have to wait some time before we can start our link plugin.
            user_id_for_link_plugin_ = info.avatar_id;
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
        link_plugin_->SetUserIdentity(user_id_for_link_plugin_);
        link_plugin_->SetContextId(context_id_for_link_plugin_);
        link_plugin_->SetApplicationName("Naali viewer");
        link_plugin_->SetApplicationDescription("Naali viewer by realXtend project");
        link_plugin_->Start();

        if (link_plugin_->IsRunning())
        {
            QString message = QString("Mumbe link plugin started: id '%1' context '%2'").arg(user_id_for_link_plugin_).arg(context_id_for_link_plugin_);
            LogInfo(message.toStdString());
        }
        else
        {
            QString error_message = QString("Link plugin connection cannot be established. %1 ").arg(link_plugin_->GetReason());
            LogError(error_message.toStdString());
        }
    }

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
