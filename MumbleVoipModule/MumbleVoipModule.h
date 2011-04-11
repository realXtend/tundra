// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "IModule.h"

#include "MumbleVoipModuleApi.h"
#include "CommunicationsService.h"
#include "Core.h"
#include <QObject>
#include "ServerInfo.h"
#include "Settings.h"

namespace MumbleVoip
{
    class LinkPlugin;
    class Provider;
    class SettingsWidget;

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
    class MUMBLE_VOIP_MODULE_API MumbleVoipModule : public QObject, public IModule
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

        static const std::string &NameStatic() { return module_name_; } /// returns name of this module. Needed for logging.

    public slots:
        void ToggleSettingsWidget();
        Provider* GetMumbleProviver() { return in_world_voice_provider_; }

    private slots:
        void SetupSettingsWidget();

    private:
        virtual void UpdateLinkPlugin(f64 frametime);

        static std::string module_name_;     

        LinkPlugin* link_plugin_;
        Provider* in_world_voice_provider_;

        static const int LINK_PLUGIN_UPDATE_INTERVAL_MS_ = 100;
        int time_from_last_update_ms_;
        QString context_id_for_link_plugin_;
        Settings *settings_;
        SettingsWidget* settings_widget_;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
