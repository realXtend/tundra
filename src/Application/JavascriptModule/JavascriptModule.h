/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   JavascriptModule.h
    @brief  Enables Javascript execution and scripting by using QtScript. */

#pragma once

#if defined (_WINDOWS)
#if defined(JAVASCRIPT_MODULE_EXPORTS)
#define JAVASCRIPT_MODULE_API __declspec(dllexport)
#else
#define JAVASCRIPT_MODULE_API __declspec(dllimport)
#endif
#else
#define JAVASCRIPT_MODULE_API
#endif

#include "IModule.h"
#include "AttributeChangeType.h"
#include "AssetFwd.h"
#include "SceneFwd.h"
#include "JavascriptFwd.h"

#include <QVariant>

class JavascriptInstance;

/// Enables JavaScript execution and scripting by using QtScript.
/** http://qt-project.org/doc/qt-4.8/ecmascript.html */
class JAVASCRIPT_MODULE_API JavascriptModule : public IModule
{
    Q_OBJECT

public:
    JavascriptModule();
    ~JavascriptModule();

    /// Prepares script instance by registering all needed services to it.
    /** If script is part of the scene, i.e. EC_Script component is present, we add some special services.
        @param instance Script istance.
        @param comp Script component, null by default. */
    void PrepareScriptInstance(JavascriptInstance* instance, EC_Script *comp = 0) const;

public slots:
    void DumpScriptInfo();
    
    /// Executes js file.
    void RunScript(const QString &scriptFilename);

    /// Executes and arbitrary js code string.
    void RunString(const QString &codeString, const QVariantMap &context = QVariantMap());

signals:
    /// A script engine has been created
    /** The purpose of this is to allow dynamic service objects (registered with Framework::RegisterDynamicObject)
        to perform further scriptengine initialization, such as registration of new datatypes. The slot
        OnScriptEngineCreated() will be invoked on the dynamic service object, if it exists. */
    void ScriptEngineCreated(QScriptEngine* engine) const;

private:
    void Load();
    void Initialize();
    void Uninitialize();

    /// Parses the plugin startup configuration file to detect which startup scripts should be run.
    /** @return Returns a QStringList of script paths */
    QStringList ParseStartupScriptConfig() const;

    /// Startup js scripts specified on the command line via --jsplugin
    /** @return List of script paths relative to bin/jsplugins */
    QStringList StartupScripts() const;

    /// Stops and deletes startup scripts
    void UnloadStartupScripts();

    /// Parse the appname and classname from an EC_Script
    void ParseAppAndClassName(EC_Script* instance, QString& appName, QString& className) const;

    /// Find a named script application
    EC_Script* FindScriptApplication(EC_Script* instance, const QString& appName) const;

    /// Create a script class instance into a script application
    void CreateScriptObject(EC_Script* app, EC_Script* instance, const QString& className)  const;

    /// Remove a script class instance from an EC_Script
    void RemoveScriptObject(EC_Script* instance)  const;
    
    /// Create script class instances for all EC_Scripts depending on this script application
    void CreateScriptObjects(EC_Script* app)  const;

    /// Remove script class instances for all EC_Scripts depending on this script application
    void RemoveScriptObjects(JavascriptInstance* jsInstance) const;

    /// Default engine for console & commandline script execution
    QScriptEngine *engine;

    /// Engines for executing startup (possibly persistent) scripts
    std::vector<JavascriptInstance *> startupScripts_;

private slots:
    /// (Re)loads and executes startup scripts.
    void LoadStartupScripts();
    void ScriptEvaluated() const;
    void ScriptUnloading() const;

    void OnSceneCreated(Scene *scene) const;
    void ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change) const;
    void ComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change) const;
    void ScriptAssetsChanged(const std::vector<ScriptAssetPtr>& newScripts);
    void ScriptAppNameChanged(const QString& newAppName) const;
    void ScriptClassNameChanged(const QString& newClassName) const;
};
