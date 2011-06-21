/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptModule.cpp
 *  @brief  Enables Javascript execution and scripting by using QtScript.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include "JavascriptInstance.h"
#include "ScriptCoreTypeDefines.h"
#include "Profiler.h"
#include "Application.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "AssetAPI.h"
#include "EC_Script.h"
#include "ScriptAssetFactory.h"
#include "EC_DynamicComponent.h"
#include "Scene.h"
#include "InputAPI.h"
#include "AudioAPI.h"
#include "FrameAPI.h"
#include "PluginAPI.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"

#include "ScriptAsset.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <QtScript>
#include <QDomElement>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

JavascriptModule *javascriptModuleInstance_ = 0;

JavascriptModule::JavascriptModule()
:IModule("Javascript"),
engine(new QScriptEngine(this))
{
}

JavascriptModule::~JavascriptModule()
{
}

void JavascriptModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Script>));
    framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new ScriptAssetFactory));
}

void JavascriptModule::Initialize()
{
    connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));

    LogInfo("Module " + Name() + " initializing...");

    assert(!javascriptModuleInstance_);
    javascriptModuleInstance_ = this;

    engine->globalObject().setProperty("print", engine->newFunction(Print));
}

void JavascriptModule::PostInitialize()
{
    RegisterCoreMetaTypes();

    framework_->Console()->RegisterCommand(
        "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)", 
        this, SLOT(ConsoleRunString()));

    framework_->Console()->RegisterCommand(
        "JsLoad", "Execute a javascript file. JsLoad(myjsfile.js)",
        this, SLOT(ConsoleRunFile()));
    
    framework_->Console()->RegisterCommand(
        "JsReloadScripts", "Reloads and re-executes startup scripts.",
        this, SLOT(ConsoleReloadScripts()));

    // Initialize startup scripts
    LoadStartupScripts();

    const boost::program_options::variables_map &programOptions = framework_->ProgramOptions();

    if (programOptions.count("run"))
    {
        commandLineStartupScript_ = programOptions["run"].as<std::string>();
        JavascriptInstance *jsInstance = new JavascriptInstance(commandLineStartupScript_.c_str(), this);
        PrepareScriptInstance(jsInstance);
        startupScripts_.push_back(jsInstance);
        jsInstance->Run();
    }
}

void JavascriptModule::Uninitialize()
{
    UnloadStartupScripts();
}

void JavascriptModule::Update(f64 frametime)
{
}

JavascriptModule *JavascriptModule::GetInstance()
{
    assert(javascriptModuleInstance_);
    return javascriptModuleInstance_;
}

void JavascriptModule::RunString(const QString &codestr, const QVariantMap &context)
{
    QMapIterator<QString, QVariant> i(context);
    while(i.hasNext())
    {
        i.next();
        engine->globalObject().setProperty(i.key(), engine->newQObject(i.value().value<QObject*>()));
    }

    engine->evaluate(codestr);
}

void JavascriptModule::RunScript(const QString &scriptFileName)
{
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
    engine->evaluate(scriptFile.readAll(), scriptFileName);
    scriptFile.close();
}

void JavascriptModule::SceneAdded(const QString &name)
{
    ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    connect(scene.get(), SIGNAL(ComponentAdded(Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentAdded(Entity*, IComponent*, AttributeChange::Type)));
    connect(scene.get(), SIGNAL(ComponentRemoved(Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentRemoved(Entity*, IComponent*, AttributeChange::Type)));
}

void JavascriptModule::ScriptAssetsChanged(const std::vector<ScriptAssetPtr>& newScripts)
{
    PROFILE(JSModule_ScriptAssetsChanged);

    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    assert(sender && "JavascriptModule::ScriptAssetsChanged needs to be invoked from EC_Script!");
    if (!sender)
        return;
    if (!newScripts.size())
    {
        LogError("Script asset vector was empty");
        return;
    }

    // First clean up any previous running script from EC_Script, if any exists.

    if (dynamic_cast<JavascriptInstance*>(sender->GetScriptInstance()))
        sender->SetScriptInstance(0);

    if (newScripts[0]->Name().endsWith(".js")) // We're positively using QtScript.
    {
        JavascriptInstance *jsInstance = new JavascriptInstance(newScripts, this);
        ComponentPtr comp;
        try
        {
            comp = sender->shared_from_this();
        } catch(...)
        {
            LogError("Couldn't update component name, cause parent entity was null.");
            return;
        }

        jsInstance->SetOwnerComponent(comp);
        sender->SetScriptInstance(jsInstance);

        // Register all core APIs and names to this script engine.
        PrepareScriptInstance(jsInstance, sender);

        if (sender->runOnLoad.Get())
        {
            sender->Run();
            
            // Now create script class instances for all EC_Scripts depending on this script application
            if (!sender->applicationName.Get().isEmpty())
                CreateScriptClassInstances(sender);
            /// \todo The script class instances are not created if RunOnLoad not specified
        }
    }
}

void JavascriptModule::ScriptAppNameChanged(const QString& newAppName)
{
    /// \todo Currently does nothing
}

void JavascriptModule::ScriptClassNameChanged(const QString& newClassName)
{
    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    assert(sender && "JavascriptModule::ScriptClassNameChanged needs to be invoked from EC_Script!");
    if (!sender)
        return;
    

    // It is possible that we do not find the script application yet. In that case, the object will be created once the app loads.
    QString appName, className;
    ParseAppAndClassName(sender, appName, className);
    EC_Script* app = FindScriptApplication(sender, appName);
    if (app)
        CreateScriptClassInstance(app, sender, className);
    else
        // If we did not find the class yet, delete the existing object in any case
        RemoveScriptClassInstance(sender);
    
}

void JavascriptModule::ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
    {
        connect(comp, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), this, SLOT(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), Qt::UniqueConnection);
        connect(comp, SIGNAL(ApplicationNameChanged(const QString&)), this, SLOT(ScriptAppNameChanged(const QString&)), Qt::UniqueConnection);
        connect(comp, SIGNAL(ClassNameChanged(const QString&)), this, SLOT(ScriptClassNameChanged(const QString&)), Qt::UniqueConnection);
    }
}

void JavascriptModule::ComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
    {
        disconnect(comp, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), this, SLOT(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)));
        disconnect(comp, SIGNAL(ApplicationNameChanged(const QString&)), this, SLOT(ScriptAppNameChanged(const QString&)));
        disconnect(comp, SIGNAL(ClassNameChanged(const QString&)), this, SLOT(ScriptClassNameChanged(const QString&)));
        EC_Script* script = dynamic_cast<EC_Script*>(comp);
        if (script)
        {
            // If the component has an existing script class instance, delete it now
            RemoveScriptClassInstance(script);
        }
    }
}

EC_Script* JavascriptModule::FindScriptApplication(EC_Script* instance, const QString& appName)
{
    if (!instance || appName.isEmpty())
        return 0;
    Entity* entity = instance->ParentEntity();
    if (!entity)
        return 0;
    Scene* scene = entity->ParentScene();
    if (!scene)
        return 0;
    // Get all script components that possibly refer to this application
    EntityList entities = scene->GetEntitiesWithComponent(EC_Script::TypeNameStatic());
    for (EntityList::iterator i = entities.begin(); i != entities.end(); ++i)
    {
        Entity* entity = i->get();
        Entity::ComponentVector comps = entity->GetComponents(EC_Script::TypeNameStatic());
        for (unsigned j = 0; j < comps.size(); ++j)
        {
            EC_Script* app = dynamic_cast<EC_Script*>(comps[j].get());
            if (app)
            {
                const QString& name = app->applicationName.Get();
                if (!name.isEmpty() && !name.trimmed().compare(appName, Qt::CaseInsensitive))
                    return app;
            }
        }
    }
    
    return 0;
}

void JavascriptModule::ParseAppAndClassName(EC_Script* instance, QString& appName, QString& className)
{
    if (!instance)
    {
        appName.clear();
        className.clear();
    }
    else
    {
        QStringList strings = instance->className.Get().split(".");
        if (strings.size() == 2)
        {
            appName = strings[0].trimmed();
            className = strings[1].trimmed();
        }
        else
        {
            appName.clear();
            className.clear();
        }
    }
}

void JavascriptModule::RemoveScriptClassInstance(EC_Script* instance)
{
    EC_Script* app = dynamic_cast<EC_Script*>(instance->scriptApplication_.lock().get());
    if (!app)
        return;
        
    JavascriptInstance* inst = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    if (!inst || !inst->IsEvaluated())
        return;
    
    QScriptEngine* appEngine = inst->GetEngine();
    QScriptValue globalObject = appEngine->globalObject();
   
    // Get the object container that holds the created script class instances from this application
    QScriptValue objectContainer = globalObject.property("scriptObjects");
    // If no object container created, no need to delete anything
    if (!objectContainer.isObject())
        return;
    
    // Delete the existing object if any
    QString objectKey = QString::number((unsigned)instance);
    QScriptValue existingObject = objectContainer.property(objectKey);
    if (existingObject.isObject())
    {
        // If the object has a "OnScriptObjectDestroyed" function, call it
        QScriptValue destructor = existingObject.property("OnScriptObjectDestroyed");
        if (!destructor.isUndefined())
            destructor.call(existingObject);
    }
    
    objectContainer.setProperty(objectKey, QScriptValue());
    
    // Forget the application
    instance->scriptApplication_.reset();
}

void JavascriptModule::CreateScriptClassInstances(EC_Script* app)
{
    JavascriptInstance* inst = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    // We don't log an error here, as it is possible that the script will get loaded later
    if (!inst || !inst->IsEvaluated())
        return;
    
    if (!app->GetScriptInstance())
        return;
    
    const QString& thisAppName = app->applicationName.Get().trimmed();
    
    Entity* appEntity = app->ParentEntity();
    if (!appEntity)
        return;
    Scene* scene = appEntity->ParentScene();
    if (!scene)
        return;
    QString appName, className;
    // Get all script components that possibly refer to this application
    EntityList entities = scene->GetEntitiesWithComponent(EC_Script::TypeNameStatic());
    for (EntityList::iterator i = entities.begin(); i != entities.end(); ++i)
    {
        Entity* entity = i->get();
        Entity::ComponentVector comps = entity->GetComponents(EC_Script::TypeNameStatic());
        for (unsigned j = 0; j < comps.size(); ++j)
        {
            EC_Script* script = dynamic_cast<EC_Script*>(comps[j].get());
            if (script)
            {
                ParseAppAndClassName(script, appName, className);
                if (appName == thisAppName)
                    CreateScriptClassInstance(app, script, className);
            }
        }
    }
}

void JavascriptModule::CreateScriptClassInstance(EC_Script* app, EC_Script* instance, const QString& className)
{
    // Delete any existing object instance, possibly in another script application
    RemoveScriptClassInstance(instance);
    
    JavascriptInstance* inst = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    // We don't log an error here, as it is possible that the script will get loaded later
    if (!inst || !inst->IsEvaluated())
        return;
    QScriptEngine* appEngine = inst->GetEngine();
    QScriptValue globalObject = appEngine->globalObject();
   
    // Get the object container that holds the created script class instances from this application
    QScriptValue objectContainer = globalObject.property("scriptObjects");
    if (!objectContainer.isObject())
    {
        objectContainer = appEngine->newArray();
        globalObject.setProperty("scriptObjects", objectContainer);
    }
    
    // If there is an existing object, delete it
    QString objectKey = QString::number((unsigned)instance);
    objectContainer.setProperty(objectKey, QScriptValue());
    
    if (className.isEmpty())
        return;
    
    QScriptValue constructor = appEngine->globalObject().property(className);
    QScriptValue object;
    if (constructor.isFunction())
        object = constructor.construct();
    else
    {
        LogError("Failed to construct class " + className + " to script application " + app->applicationName.Get() + ": constructor is not a function");
        return;
    }
    if (appEngine->hasUncaughtException())
    {
        LogError("Exception in constructor for for class " + className + " in script application " + app->applicationName.Get() + ": " + object.toString());
        QStringList trace = appEngine->uncaughtExceptionBacktrace();
        QStringList::const_iterator it;
        for(it = trace.constBegin(); it != trace.constEnd(); ++it)
            LogError((*it).toLocal8Bit().constData());

        std::stringstream ss;
        int linenum = appEngine->uncaughtExceptionLineNumber();
        ss << linenum;
        LogError(ss.str());
        return;
    }
    if (!object.isObject())
    {
        LogError("Constructed object for class " + className + " in script application " + app->applicationName.Get() + " is not an object");
        return;
    }
    
    // Set the object's "me" property to refer to the class instance's entity
    object.setProperty("me", appEngine->newQObject(instance->ParentEntity()));
    
    // Store the object to the container
    objectContainer.setProperty(objectKey, object);
    
    // Remember that the component has a script object created from this application
    instance->scriptApplication_ = app->shared_from_this();
}

QStringList JavascriptModule::ParseStartupScriptConfig()
{
    QStringList pluginsToLoad;
    QString configFile = framework_->Plugins()->ConfigurationFile();

    QDomDocument doc("plugins");
    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to open file \"" + configFile + "\"!");
        return QStringList();
    }
    if (!doc.setContent(&file))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to parse XML file \"" + configFile + "\"!");
        file.close();
        return QStringList();
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == "jsplugin" && e.hasAttribute("path"))
            pluginsToLoad.push_back(e.attribute("path"));

        n = n.nextSibling();
    }
    return pluginsToLoad;
}

void JavascriptModule::LoadStartupScripts()
{
    UnloadStartupScripts();
    
    std::string path = Application::InstallationDirectory().toStdString() + "jsmodules/startup";
    std::vector<std::string> scripts;

    try
    {
        boost::filesystem::directory_iterator i(path);
        boost::filesystem::directory_iterator end_iter;
        while(i != end_iter)
        {
            if (boost::filesystem::is_regular_file(i->status()))
            {
                std::string ext = i->path().extension();
                boost::algorithm::to_lower(ext);
                if (ext == ".js")
                    scripts.push_back(i->path().string());
            }
            ++i;
        }
    }
    catch(std::exception &/*e*/)
    {
    }
    
    QStringList startupScriptsToLoad = ParseStartupScriptConfig();

    // Create a script instance for each of the files, register services for it and try to run.
    for(uint i = 0; i < scripts.size(); ++i)
    {
        QString startupScript = scripts[i].c_str();
        startupScript = startupScript.replace("\\", "/");
        QString baseName = startupScript.mid(startupScript.lastIndexOf("/")+1);
        if (startupScriptsToLoad.contains(startupScript) || startupScriptsToLoad.contains(baseName))
        {
            LogInfo("Loading .js startup script \"" + baseName + "\".");
            JavascriptInstance* jsInstance = new JavascriptInstance(startupScript, this);
            PrepareScriptInstance(jsInstance);
            startupScripts_.push_back(jsInstance);
            jsInstance->Run();
        }
    }
}

void JavascriptModule::UnloadStartupScripts()
{
    // Stop the startup scripts, if any running
    for(uint i = 0; i < startupScripts_.size(); ++i)
        startupScripts_[i]->Unload();
    for(uint i = 0; i < startupScripts_.size(); ++i)
        delete startupScripts_[i];
    startupScripts_.clear();
}

void JavascriptModule::PrepareScriptInstance(JavascriptInstance* instance, EC_Script *comp)
{
    static std::set<QObject*> checked;
    
    // Register framework's dynamic properties (service objects) and the framework itself to the script engine
    QList<QByteArray> properties = framework_->dynamicPropertyNames();
    for(QList<QByteArray>::size_type i = 0; i < properties.size(); ++i)
    {
        QString name = properties[i];
        QObject* serviceobject = framework_->property(name.toStdString().c_str()).value<QObject*>();
        instance->RegisterService(serviceobject, name);
        
        if (checked.find(serviceobject) == checked.end())
        {
            // Check if the service object has an OnScriptEngineCreated() slot, and give it a chance to perform further actions
            const QMetaObject* meta = serviceobject->metaObject();
            if (meta->indexOfSlot("OnScriptEngineCreated(QScriptEngine*)") != -1)
                QObject::connect(this, SIGNAL(ScriptEngineCreated(QScriptEngine*)), serviceobject, SLOT(OnScriptEngineCreated(QScriptEngine*)));
            
            checked.insert(serviceobject);
        }
    }

    instance->RegisterService(framework_, "framework");
    instance->RegisterService(instance, "engine");
    
    for(uint i = 0; i < (uint)properties.size(); ++i)
        instance->RegisterService(framework_->property(properties[i]).value<QObject*>(), properties[i]);

    if (comp)
    {
        // Set entity and scene that own the EC_Script component.
        instance->RegisterService(comp->ParentEntity(), "me");
        instance->RegisterService(comp->ParentEntity()->ParentScene(), "scene");
    }

    emit ScriptEngineCreated(instance->GetEngine());
}

void JavascriptModule::ConsoleRunString(const QStringList &params)
{
    if (params.size() != 1)
    {
        LogError("Usage: JsExec(print 1 + 1)");
        return;
    }

    JavascriptModule::RunString(params[0]);
}

void JavascriptModule::ConsoleRunFile(const QStringList &params)
{
    if (params.size() != 1)
    {
        LogError("Usage: JsLoad(myfile.js)");
        return;
    }

    JavascriptModule::RunScript(params[0]);
}

void JavascriptModule::ConsoleReloadScripts()
{
    LoadStartupScripts();
}

QScriptValue Print(QScriptContext *context, QScriptEngine *engine)
{
    LogInfo("{QtScript} " + context->argument(0).toString());
    return QScriptValue();
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new JavascriptModule();
    fw->RegisterModule(module);
}
}
