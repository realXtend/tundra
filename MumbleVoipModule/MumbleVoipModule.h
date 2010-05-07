// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "MumbleVoipModuleApi.h"
#include "Vector3d.h"

#include <QObject>

namespace MumbleVoip
{
    class LinkPlugin;
    class ServerObserver;
    class ConnectionManager;
    class ServerInfo;

    /**
     *  Mumble support for Naali viewer.
     *
     *  Offer console commands:
     *    'mumble link(avatar_id, context_id)'
     *    'mumble unlink'
     *    'mumble start(server_url)'
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
     */
    class MUMBLE_VOIP_MODULE_API MumbleVoipModule : public QObject, public Foundation::ModuleInterfaceImpl
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

    protected:
        static std::string module_name_;

        virtual void InitializeConsoleCommands();
        virtual Console::CommandResult OnConsoleMumbleLink(const StringVector &params);
        virtual Console::CommandResult OnConsoleMumbleUnlink(const StringVector &params);
        virtual Console::CommandResult OnConsoleMumbleStart(const StringVector &params);
        virtual Console::CommandResult OnConsoleEnableVoiceActivityDetector(const StringVector &params);
        virtual Console::CommandResult OnConsoleDisableVoiceActivityDetector(const StringVector &params);

        virtual void UpdateLinkPlugin(f64 frametime);
        virtual bool GetAvatarPosition(Vector3df& position, Vector3df& direction);
        virtual bool GetCameraPosition(Vector3df& position, Vector3df& direction);

    private:
        LinkPlugin* link_plugin_;
        ServerObserver* server_observer_;
        ConnectionManager* connection_manager_;

        static const int UPDATE_TIME_MS_ = 100;
        int time_from_last_update_ms_;
        QString user_id_for_link_plugin_;
        QString context_id_for_link_plugin_;
        bool use_camera_position_; 
        bool mumble_client_started_;
        bool mumble_use_library_;
        event_category_id_t event_category_framework_;

    private slots:
        void OnMumbleServerInfoReceived(const ServerInfo &info);
        void StartLinkPlugin();
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
