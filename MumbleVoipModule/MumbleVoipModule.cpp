// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MumbleVoipModule.h"
#include "MemoryLeakCheck.h"
#include "LinkPlugin.h"
#include "RexLogicModule.h"
#include "ModuleManager.h"
#include "Avatar/Avatar.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"
#include "ConsoleCommandServiceInterface.h"
#include <QDesktopServices>

namespace MumbleVoip
{
	std::string MumbleVoipModule::module_name_ = "MumbleVoipModule";

	MumbleVoipModule::MumbleVoipModule()
        : ModuleInterfaceImpl(module_name_),
          link_plugin_(new LinkPlugin()),
          time_from_last_update_ms_(0)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(link_plugin_);
    }

    void MumbleVoipModule::Load()
    {
    }

    void MumbleVoipModule::Unload()
    {
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
        if (!link_plugin_->IsRunning())
            return; 

        time_from_last_update_ms_ += 1000*frametime;
        if (time_from_last_update_ms_ < UPDATE_TIME_MS_)
            return;
        time_from_last_update_ms_ = 0;
        
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
                OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
                Quaternion q = ogre_placeable->GetOrientation();

                Vector3df position_vector = ogre_placeable->GetPosition(); 
                Vector3df front_vector = q*Vector3df(1,0,0);
                Vector3df top_vector(0,0,1);
                link_plugin_->SetAvatarPosition(position_vector, front_vector, top_vector);
            }
        }

        Scene::EntityPtr camera = rex_logic_module->GetCameraEntity().lock();
        if (camera)
        {
            const Foundation::ComponentInterfacePtr &placeable_component = camera->GetComponent("EC_OgrePlaceable");
            if (placeable_component)
            {
                OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
                Quaternion q = ogre_placeable->GetOrientation();

                Vector3df position_vector = ogre_placeable->GetPosition(); 
                Vector3df front_vector = q*Vector3df(1,0,0);
                Vector3df top_vector(0,0,1);
                link_plugin_->SetCameraPosition(position_vector, front_vector, top_vector);
            }
        }

        link_plugin_->SendData();
    }

	bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
	{
		return false;
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
    
    Console::CommandResult  MumbleVoipModule::OnConsoleMumbleLink(const StringVector &params)
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
            StartMumbleClient(server_url);
            return Console::ResultSuccess("Mumbe client started.");
        }
        catch(std::exception &e)
        {
            QString error_message = QString("Cannot start Mumble client: %1").arg(QString(e.what()));
            return Console::ResultFailure(error_message.toStdString());        
        }
    }

    void MumbleVoipModule::StartMumbleClient(const QString& server_url)
    {
        QUrl url(server_url);
        if (!url.isValid())
        {
            QString error_message = QString("Url '%1' is invalid.").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
        }
        
        if (! QDesktopServices::openUrl(server_url))
        {
            QString error_message = QString("Cannot find handler application for url: %1").arg(server_url);
            throw std::exception(error_message.toStdString().c_str());
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
