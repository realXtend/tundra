// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"

#include "MumbleVoipModuleApi.h"
#include "MumbleFwd.h"

#include <QObject>

class QScriptEngine;

namespace MumbleVoip
{
    /**
     *  Mumble support for Tundra that provides VOIP communications.
     *
     *  Implements a mumble voice provider and a mumble voice session that can be obtained from the provider.
     *  The provider can be acquired as a dynamic Framework object/property with name "mumbleprovider".
     *
     *  Uses mumbleclient library to establish connections to Mumble servers.
     *
     *  See usage example in scenes/Mumble/
     */
    class MUMBLE_VOIP_MODULE_API MumbleVoipModule : public IModule
    {
        
    Q_OBJECT

    public:
        /// Constructor.
        MumbleVoipModule();

        /// Deconstructor.
        virtual ~MumbleVoipModule();

        /// IModule override.
        void PostInitialize();

        /// IModule override.
        void Uninitialize();

        /// IModule override.
        void Update(f64 frametime);

    public slots:
        void ToggleSettingsWidget();

    private slots:
        void OnScriptEngineCreated(QScriptEngine *engine);

    private:       
        Provider* provider_;
        SettingsWidget* settingsWidget_;
        Settings *settings_;
    };
}
