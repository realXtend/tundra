// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "MumbleVoipModuleApi.h"
#include "CommunicationsService.h"
#include "Core.h"
#include <QObject>
#include "ServerInfo.h"

namespace MumbleVoip
{
    class LinkPlugin;
    class ServerInfoProvider;
    class ConnectionManager;
    class ApplicationManager;
    class Provider;

    /**
     *  Mumble support for Naali viewer.
     *
     *  Offer console commands:
     *    'mumble link(avatar_id, context_id)'
     *    'mumble unlink'
     *    'mumble start(server_url)'
     * todo 'mumble status'
     *  
     *  Request mumble server information when user has logged to world and establish a connection
     *  to mumble server.
     *
     *  By default the nativi mumble client is used to establish the connection.
     *  If command line argument '--usemumblelibrary' has been given then mumbleclient library is used to 
     *  establish a connection.
     *
     *  In future this module will implement InWorldVoiceProvider interface and is controlled by user interface and 
     *  mumbleclient library is used to make connections.
     *
     *  command line argument '--use_native_mumble_client' disabled mumbleclient library and will use native mumble
     *  client application with link plugin.
     */
    class MUMBLE_VOIP_MODULE_API MumbleVoipModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

    public:
        MumbleVoipModule();
        virtual ~MumbleVoipModule();

        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();

        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return module_name_; } //! returns name of this module. Needed for logging.

    private slots:
        void StartLinkPlugin();
        void StartMumbleClient(ServerInfo info);

    private:
        static std::string module_name_;

        virtual void InitializeConsoleCommands();
        virtual Console::CommandResult OnConsoleMumbleLink(const StringVector &params);
        virtual Console::CommandResult OnConsoleMumbleUnlink(const StringVector &params);
        virtual Console::CommandResult OnConsoleMumbleStart(const StringVector &params);
        virtual Console::CommandResult OnConsoleMumbleStats(const StringVector &params);

        virtual void UpdateLinkPlugin(f64 frametime);
        virtual bool GetAvatarPosition(Vector3df& position, Vector3df& direction);
        virtual bool GetCameraPosition(Vector3df& position, Vector3df& direction);
        
        LinkPlugin* link_plugin_;
        ServerInfoProvider* server_info_provider_;
        Provider* in_world_voice_provider_;

        static const int LINK_PLUGIN_UPDATE_INTERVAL_MS_ = 100;
        int time_from_last_update_ms_;
        bool use_camera_position_; 
        bool use_native_mumble_client_;
        event_category_id_t event_category_framework_;
        QString avatar_id_for_link_plugin_;
        QString context_id_for_link_plugin_;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
