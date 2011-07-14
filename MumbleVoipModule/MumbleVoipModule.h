// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "MumbleVoipModuleApi.h"
#include "CommunicationsService.h"
#include "Core.h"
#include <QObject>
#include "ServerInfo.h"
#include "Settings.h"

namespace MumbleVoip
{
    class Provider;
    class SettingsWidget;

    /**
     *  Mumble support for Tundra.
     *
     *  Implements InWorldVoiceProvider and InWorldVoiceSession interfaces
     *  to provide VOIP communications.
     *
     *  Uses mumbleclient library to establish connections to Mumble servers.
     *
     *  See usage example in scenes/Mumble/
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
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return module_name_; } //! returns name of this module. Needed for logging.

    private slots:
        void SetupSettingsWidget();

    private:
        static std::string module_name_;

        virtual void InitializeConsoleCommands();
        
        Provider* in_world_voice_provider_;
        event_category_id_t event_category_framework_;
        Settings settings_;
        SettingsWidget* settings_widget_;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
