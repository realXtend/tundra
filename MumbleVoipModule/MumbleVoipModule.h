// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "IModule.h"
#include "MumbleVoipModuleApi.h"
#include "CommunicationsService.h"
#include "Core.h"
#include "ServerInfo.h"

#include <QObject>

namespace MumbleVoip
{
    class LinkPlugin;
    class Provider;
    class Settings;
    class SettingsWidget;

    /// Mumble support for Tundra. Look more from MumbleVoip::Provider and its Session object.
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

    public slots:
        void ToggleSettingsWidget();
        Provider* GetMumbleProviver() { return in_world_voice_provider_; }

    private slots:
        void SetupSettingsWidget();

    private:
        virtual void UpdateLinkPlugin(f64 frametime);

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
