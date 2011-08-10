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
#include "TundraLogicModule.h"

#include "ScriptAsset.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <QtScript>
#include <QDomElement>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

JavascriptModule::JavascriptModule() :
    IModule("Javascript"),
    engine(new QScriptEngine(this))
{
}

JavascriptModule::~JavascriptModule()
{
}

void JavascriptModule::Load()
{
    if (!framework_->Scene()->IsComponentFactoryRegistered(EC_Script::TypeNameStatic()))
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Script>));
    if (!framework_->Asset()->IsAssetTypeFactoryRegistered("Script"))
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new ScriptAssetFactory));
}

void JavascriptModule::Initialize()
{
    connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));
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
    if (newScripts.empty())
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

        jsInstance->SetOwner(comp);
        sender->SetScriptInstance(jsInstance);

        // Register all core APIs and names to this script engine.
        PrepareScriptInstance(jsInstance, sender);

        // If this component is a script application, connect to the evaluate / unload signals so that we can create or delete script objects as needed
        if (!sender->applicationName.Get().trimmed().isEmpty())
        {
            connect(jsInstance, SIGNAL(ScriptEvaluated()), this, SLOT(ScriptEvaluated()), Qt::UniqueConnection);
            connect(jsInstance, SIGNAL(ScriptUnloading()), this, SLOT(ScriptUnloading()), Qt::UniqueConnection);
        }
        
        bool isApplication = !sender->applicationName.Get().trimmed().isEmpty();
        if (sender->runOnLoad.Get() && sender->ShouldRun())
        {
            if (isApplication && framework_->ProgramOptions().count("disablerunonload") > 0)
                return;
            jsInstance->Run();
        }
    }
}

void JavascriptModule::ScriptAppNameChanged(const QString& newAppName)
{
    /// \todo Currently we do not react to changing the script app name on the fly.
}

void JavascriptModule::ScriptClassNameChanged(const QString& newClassName)
{
    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    assert(sender && "JavascriptModule::ScriptClassNameChanged needs to be invoked from EC_Script!");
    if (!sender)
        return;
    
    // Check runmode for the object
    if (!sender->ShouldRun())
        return;
    
    // It is possible that we do not find the script application yet. In that case, the object will be created once the app loads.
    QString appName, className;
    ParseAppAndClassName(sender, appName, className);
    EC_Script* app = FindScriptApplication(sender, appName);
    if (app)
        CreateScriptObject(app, sender, className);
    else
        // If we did not find the class yet, delete the existing object in any case
        RemoveScriptObject(sender);
    
}

void JavascriptModule::ScriptEvaluated()
{
    JavascriptInstance* sender = dynamic_cast<JavascriptInstance*>(this->sender());
    if (!sender)
        return;
    EC_Script* app = dynamic_cast<EC_Script*>(sender->Owner().lock().get());
    if (app)
        CreateScriptObjects(app);
}

void JavascriptModule::ScriptUnloading()
{
    JavascriptInstance* sender = dynamic_cast<JavascriptInstance*>(this->sender());
    if (!sender)
        return;
    RemoveScriptObjects(sender);
}

void JavascriptModule::ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
    {
        connect(comp, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), this, SLOT(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), Qt::UniqueConnection);
        connect(comp, SIGNAL(ApplicationNameChanged(const QString&)), this, SLOT(ScriptAppNameChanged(const QString&)), Qt::UniqueConnection);
        connect(comp, SIGNAL(ClassNameChanged(const QString&)), this, SLOT(ScriptClassNameChanged(const QString&)), Qt::UniqueConnection);
        // Set the script component's isClient & isServer flags to determine run mode
        EC_Script* script = checked_static_cast<EC_Script*>(comp);
        TundraLogic::TundraLogicModule *tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        bool isServer = tundra->IsServer();
        bool isClient = !isServer;
        /// \hack Currently we use the scene's name to recognize the single-user scene: it will have both client and server flags set
        if (!isServer)
        {
            Scene* scene = entity->ParentScene();
            if (scene)
                isServer = scene->Name().compare("TundraServer", Qt::CaseInsensitive) == 0;
        }
        script->SetIsClientIsServer(isClient, isServer);
    }
}

void JavascriptModule::ComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
    {
        disconnect(comp, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), this, SLOT(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)));
        disconnect(comp, SIGNAL(ApplicationNameChanged(const QString&)), this, SLOT(ScriptAppNameChanged(const QString&)));
        disconnect(comp, SIGNAL(ClassNameChanged(const QString&)), this, SLOT(ScriptClassNameChanged(const QString&)));
        EC_Script* script = checked_static_cast<EC_Script*>(comp);
        // If the component has an existing script class instance, delete it now
        RemoveScriptObject(script);
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

void JavascriptModule::CreateScriptObject(EC_Script* app, EC_Script* instance, const QString& className)
{
    // Delete any existing object instance, possibly in another script application
    RemoveScriptObject(instance);
    
    // Make sure the runmode allows object creation
    if (!instance->ShouldRun())
        return;
    
    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    if (!jsInstance || !jsInstance->IsEvaluated())
        return;
    
    QScriptEngine* appEngine = jsInstance->Engine();
    QScriptValue globalObject = appEngine->globalObject();
   
    // Get the object container that holds the created script class instances from this application
    QScriptValue objectContainer = globalObject.property("scriptObjects");
    if (!objectContainer.isObject())
    {
        objectContainer = appEngine->newArray();
        globalObject.setProperty("scriptObjects", objectContainer);
    }
    
    if (className.isEmpty())
        return;
    
    const QString& appAndClassName = instance->className.Get();
    QScriptValue constructor = appEngine->globalObject().property(className);
    QScriptValue object;
    if (constructor.isFunction())
    {
        // Send the entity and instance EC_Script as parameters to the constructor
        QScriptValueList parameters;
        parameters.push_back(appEngine->newQObject(instance->ParentEntity()));
        parameters.push_back(appEngine->newQObject(instance));
        object = constructor.construct(parameters);
    }
    else
    {
        LogError("Constructor for " + appAndClassName + " not found");
        return;
    }
    if (jsInstance->CheckAndPrintException("In object " + appAndClassName + " constructor: ", object))
        return; // There was an exception calling the constructor.
    if (!object.isObject())
    {
        LogError("Constructor for " + appAndClassName + " did not return an object");
        return;
    }
    
    // Store the object to the container
    objectContainer.setProperty(instance->GetScriptObjectKey(), object);
    
    // Remember that the component has a script object created from this application
    instance->SetScriptApplication(app);
}

void JavascriptModule::RemoveScriptObject(EC_Script* instance)
{
    EC_Script* app = instance->GetScriptApplication();
    if (!app)
        return;
    
    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    if (!jsInstance || !jsInstance->IsEvaluated())
        return;
    
    QScriptEngine* appEngine = jsInstance->Engine();
    QScriptValue globalObject = appEngine->globalObject();
   
    // Get the object container that holds the created script class instances from this application
    QScriptValue objectContainer = globalObject.property("scriptObjects");
    // If no object container created, no need to delete anything
    if (!objectContainer.isObject())
        return;
    
    // Delete the existing object if any
    QString objectKey = instance->GetScriptObjectKey();
    QScriptValue existingObject = objectContainer.property(objectKey);
    if (existingObject.isObject())
    {
        // If the object has a "OnScriptObjectDestroyed" function, call it
        QScriptValue destructor = existingObject.property("OnScriptObjectDestroyed");
        if (!destructor.isUndefined())
        {
            QScriptValue result = destructor.call(existingObject);
            const QString& appAndClassName = instance->className.Get();
            jsInstance->CheckAndPrintException("In object " + appAndClassName + " destructor: ", result);
        }
    }
    
    objectContainer.setProperty(objectKey, QScriptValue());
    
    // Forget the application
    instance->SetScriptApplication(0);
}

void JavascriptModule::CreateScriptObjects(EC_Script* app)
{
    // If application has an empty name, we can not create script objects out of it. Skip the expensive
    // entity/scriptcomponent scan in that case.
    const QString& thisAppName = app->applicationName.Get().trimmed();
    if (thisAppName.isEmpty())
        return;

    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->GetScriptInstance());
    if (!jsInstance || !jsInstance->IsEvaluated())
    {
        LogError("CreateScriptObjects: the application EC_Script does not have a script engine that has already evaluated its code");
        return;
    }
    
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
            if (script && script->ShouldRun())
            {
                ParseAppAndClassName(script, appName, className);
                if (appName == thisAppName)
                    CreateScriptObject(app, script, className);
            }
        }
    }
}

void JavascriptModule::RemoveScriptObjects(JavascriptInstance* jsInstance)
{
    QScriptEngine* appEngine = jsInstance->Engine();
    if (!appEngine)
        return;
    
    QScriptValue globalObject = appEngine->globalObject();
    
    // Get the object container that holds the created script class instances from this application
    QScriptValue objectContainer = globalObject.property("scriptObjects");
    // If no object container created, no need to delete anything
    if (!objectContainer.isObject())
        return;
    
    // Delete all existing script objects
    QScriptValueIterator it(objectContainer);
    while (it.hasNext())
    {
        it.next();
        QScriptValue existingObject = it.value();
        if (existingObject.isObject())
        {
            // If the object has a "OnScriptObjectDestroyed" function, call it
            QScriptValue destructor = existingObject.property("OnScriptObjectDestroyed");
            if (!destructor.isUndefined())
            {
                QScriptValue result = destructor.call(existingObject);
                jsInstance->CheckAndPrintException("In script object destructor: ", result);
            }
        }
    }
    
    // Empty the container
    globalObject.setProperty("scriptObjects", QScriptValue());
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
    LogInfo(Name() + ": Loading startup scripts");
    for(uint i = 0; i < scripts.size(); ++i)
    {
        QString startupScript = scripts[i].c_str();
        startupScript = startupScript.replace("\\", "/");
        QString baseName = startupScript.mid(startupScript.lastIndexOf("/")+1);
        if (startupScriptsToLoad.contains(startupScript) || startupScriptsToLoad.contains(baseName))
        {
            LogInfo(Name() + ": ** " + baseName.toStdString());
            JavascriptInstance* jsInstance = new JavascriptInstance(startupScript, this);
            PrepareScriptInstance(jsInstance);
            startupScripts_.push_back(jsInstance);
            jsInstance->Run();

            // Remove from the list so we can check relative paths next.
            startupScriptsToLoad.removeAll(startupScript);
            startupScriptsToLoad.removeAll(baseName);
        }
    }

    // Allow relative paths from '/<install_dir>' and '/<install_dir>/jsmodules'  to start also
    QDir jsPluginsDir("./jsmodules");
    foreach(QString startupScript, startupScriptsToLoad)
    {
        // Only allow relative paths, maybe allow absolute paths as well, maybe even URLs at some point?
        if (!QDir::isRelativePath(startupScript))
            continue;

        QString pathToFile;
        if (jsPluginsDir.exists(startupScript))
            pathToFile = jsPluginsDir.filePath(startupScript);
        else if (QFile::exists(startupScript))
            pathToFile = startupScript;
        else
        {
            LogWarning(Name() + "** Could not find startup file for: " + startupScript.toStdString());
            continue;
        }

        LogInfo(Name() + ": ** " + startupScript.toStdString());
        JavascriptInstance* jsInstance = new JavascriptInstance(pathToFile, this);
        PrepareScriptInstance(jsInstance);
        startupScripts_.push_back(jsInstance);
        jsInstance->Run();
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

    emit ScriptEngineCreated(instance->Engine());
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
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new JavascriptModule();
    fw->RegisterModule(module);
}
}
