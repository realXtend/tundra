// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "MumbleVoipModuleApi.h"

#include <QObject>
#include "ServerInfo.h"

namespace MumbleVoip
{
    class LinkPlugin;
    class ServerObserver;
    class ConnectionManager;

    /**
	 *  Mumble support for Naali viewer.
	 *
     *  Offer console commands:
     *    'mumble link(avatar_id, context_id)'
     *    'mumble unlink'
     *    'mumble start(server_url)'
     *  
     *  Starts mumble client with link plugin when user logins to world server.
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
        virtual Console::CommandResult  OnConsoleMumbleLink(const StringVector &params);
        virtual Console::CommandResult  OnConsoleMumbleUnlink(const StringVector &params);
        virtual Console::CommandResult  OnConsoleMumbleStart(const StringVector &params);

        virtual void UpdateLinkPlugin(f64 frametime);

		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Unknown;
    private:

        LinkPlugin* link_plugin_;
        ServerObserver* server_observer_;
        ConnectionManager* connection_manager_;

        static const int UPDATE_TIME_MS_ = 100;
        int time_from_last_update_ms_;
        QString user_id_for_link_plugin_;
        QString context_id_for_link_plugin_;

    private slots:
        void OnMumbleServerInfoReceived(ServerInfo info);
        void StartLinkPlugin();
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
