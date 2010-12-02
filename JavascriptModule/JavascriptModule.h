/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptModule.h
 *  @brief  Enables Javascript execution and scripting in Naali.
 */

#ifndef incl_JavascriptModule_JavascriptModule_h
#define incl_JavascriptModule_JavascriptModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "AttributeChangeType.h"
#include "ScriptServiceInterface.h"
#include "AssetFwd.h"
#include "ScriptAsset.h"

#include <QObject>

class QScriptEngine;
class QScriptContext;
class QScriptEngine;
class QScriptValue;

class JavascriptInstance;
class EC_Script;

/// Enables Javascript execution and scripting in Naali.
/**
*/
class JavascriptModule : public QObject, public IModule, public Foundation::ScriptServiceInterface
{
    Q_OBJECT

public:
    /// Default constructor.
    JavascriptModule();

    /// Destructor.
    ~JavascriptModule();

    /// IModule override.
    void Load();

    /// IModule override.
    void PreInitialize();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void PostInitialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

    /// Returns the currently initialized JavascriptModule.
    static JavascriptModule *GetInstance();

    void RunScript(const QString &scriptname);
    void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

    Console::CommandResult ConsoleRunString(const StringVector &params);
    Console::CommandResult ConsoleRunFile(const StringVector &params);
    Console::CommandResult ConsoleReloadScripts(const StringVector &params);

    /// Prepares script instance by registering all needed services to it.
    /** If script is part of the scene, i.e. EC_Script component is present, we add some special services.
        @param instance Script istance.
        @param comp Script component, null by default.
    */
    void PrepareScriptInstance(JavascriptInstance* instance, EC_Script *comp = 0);

public slots:
    //! New scene has been added to foundation.
    void SceneAdded(const QString &name);

    void ScriptAssetChanged(ScriptAssetPtr newScript);

    //! New component has been added to scene.
    void ComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);

    //! Component has been removed from scene.
    void ComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);

signals:
    //! A script engine has been created
    /*! The purpose of this is to allow dynamic service objects (registered with FrameWork->RegisterDynamicObject())
        to perform further scriptengine initialization, such as registration of new datatypes. The slot
        OnScriptEngineCreated() will be invoked on the dynamic service object, if it exists.
     */
    void ScriptEngineCreated(QScriptEngine* engine);

private:
    //! Load & execute startup scripts
    /*! Destroys old scripts if they exist
     */
    void LoadStartupScripts();
    
    //! Stop & delete startup scripts
    void UnloadStartupScripts();
    
    /// Type name of the module.
    static std::string type_name_static_;
    
    /// Default engine for console & commandline script execution
    QScriptEngine *engine;
    
    /// Engines for executing startup (possibly persistent) scripts
    std::vector<JavascriptInstance *> startupScripts_;
    
    /// Additional startupscript defined from command line
    std::string commandLineStartupScript_;
    
    /// Framework event category
    event_category_id_t frameworkEventCategory_;
};

//api stuff
QScriptValue Print(QScriptContext *context, QScriptEngine *engine);
QScriptValue ScriptRunFile(QScriptContext *context, QScriptEngine *engine);

#endif
