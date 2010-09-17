/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptModule.h
 *  @brief  
 */

#ifndef incl_JavascriptModule_JavascriptModule_h
#define incl_JavascriptModule_JavascriptModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "AttributeChangeType.h"
#include "ScriptServiceInterface.h"

#include <QObject>

class QScriptEngine;
class QScriptContext;
class QScriptEngine;
class QScriptValue;

/**
*/
class JavascriptModule : public QObject, public Foundation::ModuleInterface, public Foundation::ScriptServiceInterface
{
    Q_OBJECT

public:
    /// Default constructor.
    JavascriptModule();

    /// Destructor.
    ~JavascriptModule();

    /// ModuleInterface override.
    void Load();

    /// ModuleInterface override.
    void PreInitialize();

    /// ModuleInterface override.
    void Initialize();

    /// ModuleInterface override.
    void PostInitialize();

    /// ModuleInterface override.
    void Uninitialize();

    /// ModuleInterface override.
    void Update(f64 frametime);

    /// ModuleInterface override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

    /// Returns the currently initialized JavascriptModule.
    static JavascriptModule *GetInstance();

    void RunScript(const QString &scriptname);
    void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

    Console::CommandResult ConsoleRunString(const StringVector &params);
    Console::CommandResult ConsoleRunFile(const StringVector &params);

public slots:
    //! New scene has been added to foundation.
    void SceneAdded(const QString &name);

    //! EC_Script script source ref has changed.
    void ScriptChanged(const QString &scriptRef);

    //! New component has been added to scene.
    void ComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);

    //! Component has been removed from scene.
    void ComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);

private:
    InputContextPtr input_;
    /// Type name of the module.
    static std::string type_name_static_;
    typedef QMap<QString, QObject*> ServiceMap;
    ServiceMap services_;

    QScriptEngine *engine;
};

//api stuff
QScriptValue Print(QScriptContext *context, QScriptEngine *engine);
QScriptValue ScriptRunFile(QScriptContext *context, QScriptEngine *engine);

#endif
