// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "CoreDefines.h"
#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"

#include "MumbleVoipModule.h"
#include "IMumble.h"
#include "Provider.h"
#include "MumbleLibrary.h"
#include "SettingsWidget.h"
#include "Settings.h"
#include "ConsoleAPI.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include <QScriptEngine>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(MumbleVoip::ISession*)
Q_DECLARE_METATYPE(MumbleVoip::ISession::State)
Q_DECLARE_METATYPE(MumbleVoip::IProvider*)
Q_DECLARE_METATYPE(MumbleVoip::IParticipant*)

namespace MumbleVoip
{
    MumbleVoipModule::MumbleVoipModule() :
        IModule("MumbleVoip"),
        provider_(0),
        settingsWidget_(0),
        settings_(0)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(provider_);
        SAFE_DELETE(settingsWidget_);
        SAFE_DELETE(settings_);
    }

    void MumbleVoipModule::PostInitialize()
    {
        settings_ = new Settings(framework_);
        provider_ = new Provider(framework_, settings_);

        if (!framework_->IsHeadless())
        {
            settingsWidget_ = new SettingsWidget(provider_, settings_);
            settingsWidget_->setWindowFlags(Qt::Tool);
            settingsWidget_->resize(1,1);
        }

        // Get JS module for registering our custom QObjects
        JavascriptModule *jsModule = framework_->GetModule<JavascriptModule>();
        if (jsModule)
            connect(jsModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
        else
            LogWarning("MumbleVoipModule: Could not get JavascriptModule to connect to the engine created signal!");
    }

    void MumbleVoipModule::Uninitialize()
    {
        SAFE_DELETE(provider_);
        MumbleLib::MumbleLibrary::Stop();
    }

    void MumbleVoipModule::Update(f64 frametime)
    {   
        if (provider_)
            provider_->Update(frametime);
    }

    void MumbleVoipModule::OnScriptEngineCreated(QScriptEngine *engine)
    {
        qScriptRegisterQObjectMetaType<MumbleVoip::ISession*>(engine);
        qScriptRegisterQObjectMetaType<MumbleVoip::IProvider*>(engine);
        qScriptRegisterQObjectMetaType<MumbleVoip::IParticipant*>(engine);

        qScriptRegisterMetaType(engine, toScriptValueEnum<MumbleVoip::ISession::State>, 
                                        fromScriptValueEnum<MumbleVoip::ISession::State>);
    }

    void MumbleVoipModule::ToggleSettingsWidget()
    {
        if (settingsWidget_)
            settingsWidget_->setVisible(!settingsWidget_->isVisible());
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new MumbleVoip::MumbleVoipModule();
        fw->RegisterModule(module);
    }
}
