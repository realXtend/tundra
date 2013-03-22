/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   JavascriptModule.cpp
    @brief  Enables Javascript execution and scripting by using QtScript. */

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
#include "GenericAssetFactory.h"
#include "EC_Script.h"
#include "ScriptAsset.h"
#include "EC_DynamicComponent.h"
#include "Scene/Scene.h"
#include "InputAPI.h"
#include "AudioAPI.h"
#include "FrameAPI.h"
#include "PluginAPI.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"
#include "TundraLogicModule.h"
#include "LoggingFunctions.h"
#include "FileUtils.h"

#include <QtScript>
#include <QDomElement>

#include "StaticPluginRegistry.h"

#include "MemoryLeakCheck.h"

JavascriptModule::JavascriptModule() :
    IModule("Javascript"),
    engine(new QScriptEngine(this))
{
}

JavascriptModule::~JavascriptModule()
{
    SAFE_DELETE(engine);
}

void JavascriptModule::Load()
{
    if (!framework_->Scene()->IsComponentFactoryRegistered(EC_Script::TypeNameStatic()))
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Script>));

    // This check is done as both js and py modules can register this factory. 
    // They both need to register .js and .py extensions to play nice.
    // @todo Refactor to separate the .js and .py assets to be in separate factories.
    if (!framework_->Asset()->IsAssetTypeFactoryRegistered("Script"))
    {
        QStringList scriptExtensions;
        scriptExtensions << ".js" << ".py";
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<ScriptAsset>("Script", scriptExtensions)));
    }
}

void JavascriptModule::Initialize()
{
    connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));

    RegisterCoreMetaTypes();

    framework_->Console()->RegisterCommand(
        "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)",
        this, SLOT(RunString(const QString &)));

    framework_->Console()->RegisterCommand(
        "JsLoad", "Execute a javascript file. JsLoad(myJsFile.js)",
        this, SLOT(RunScript(const QString &)));

    framework_->Console()->RegisterCommand(
        "JsReloadScripts", "Reloads and re-executes startup scripts.",
        this, SLOT(LoadStartupScripts()));
        
    framework_->Console()->RegisterCommand(
        "JsDumpInfo", "Dumps all EC_Script information to console",
        this, SLOT(DumpScriptInfo()));

    // Initialize startup scripts
    LoadStartupScripts();

    foreach(const QString &script, framework_->CommandLineParameters("--run"))
    {
        JavascriptInstance *jsInstance = new JavascriptInstance(script, this);
        PrepareScriptInstance(jsInstance);
        startupScripts_.push_back(jsInstance);
        jsInstance->Run();
    }
}

void JavascriptModule::Uninitialize()
{
    UnloadStartupScripts();
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

void JavascriptModule::DumpScriptInfo()
{
    Scene *scene = framework_->Scene()->MainCameraScene();
    if (!scene)
    {
        LogInfo("Scene is null, no scripts running.");
        return;
    }
    std::vector<shared_ptr<EC_Script> > scripts = scene->Components<EC_Script>();
    if (scripts.empty())
    {
        LogInfo("No scripts running in the scene");
        return;
    }
    for(unsigned i = 0; i < scripts.size(); ++i)
    {
        JavascriptInstance *jsInstance = dynamic_cast<JavascriptInstance*>(scripts[i]->ScriptInstance());
        if (!jsInstance)
            continue;
        if (!jsInstance->IsEvaluated())
            continue;
            
        QMap<QString, uint> dump = jsInstance->DumpEngineInformation();
        
        QString dumpStr = QString("QScriptValues %1 Objects %10 IsNull %2 Numbers %3 Bools %4 Strings %5 Arrays %6 Functions %7 QObjects %8 QObjMethods %9")
            .arg(dump["QScriptValues"], 3, 10, QLatin1Char('0')).arg(dump["Is null"], 3, 10, QLatin1Char(' ')).arg(dump["Numbers"], 3, 10, QLatin1Char('0'))
            .arg(dump["Booleans"], 3, 10, QLatin1Char('0')).arg(dump["Strings"], 3, 10, QLatin1Char('0')).arg(dump["Arrays"], 3, 10, QLatin1Char('0'))
            .arg(dump["Functions"], 3, 10, QLatin1Char('0')).arg(dump["QObjects"], 3, 10, QLatin1Char('0')).arg(dump["QObject methods"], 3, 10, QLatin1Char('0'))
            .arg(dump["Objects"], 3, 10, QLatin1Char('0'));

        LogInfo(scripts[i]->ParentEntity()->ToString() + " { " + dumpStr + " }");
    }
}

void JavascriptModule::RunScript(const QString &scriptFileName)
{
    QFile scriptFile(scriptFileName);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError("JavascriptModule::RunScript: failed to open file: " + scriptFileName);
        return;
    }

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

    if (dynamic_cast<JavascriptInstance*>(sender->ScriptInstance()))
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
            if (isApplication && framework_->HasCommandLineParameter("--disablerunonload"))
                return;
            jsInstance->Run();
        }
    }
}

void JavascriptModule::ScriptAppNameChanged(const QString& /*newAppName*/)
{
    /// \todo Currently we do not react to changing the script app name on the fly.
}

void JavascriptModule::ScriptClassNameChanged(const QString& newClassName)
{
    UNREFERENCED_PARAM(newClassName) /**< @todo Do we want to do something with this? */
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

void JavascriptModule::ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type /*change*/)
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

void JavascriptModule::ComponentRemoved(Entity* /*entity*/, IComponent* comp, AttributeChange::Type /*change*/)
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
    std::vector<shared_ptr<EC_Script> > scripts = scene->Components<EC_Script>();
    for(unsigned i = 0; i < scripts.size(); ++i)
    {
        const QString& name = scripts[i]->applicationName.Get();
        if (!name.isEmpty() && !name.trimmed().compare(appName, Qt::CaseInsensitive))
            return scripts[i].get();
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
    
    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->ScriptInstance());
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
    objectContainer.setProperty(instance->ScriptObjectKey(), object);
    
    // Remember that the component has a script object created from this application
    instance->SetScriptApplication(app);
}

void JavascriptModule::RemoveScriptObject(EC_Script* instance)
{
    EC_Script* app = instance->ScriptApplication();
    if (!app)
        return;
    
    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->ScriptInstance());
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
    QString objectKey = instance->ScriptObjectKey();
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

    JavascriptInstance* jsInstance = dynamic_cast<JavascriptInstance*>(app->ScriptInstance());
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
    std::vector<shared_ptr<EC_Script> > scripts = scene->Components<EC_Script>();
    for(unsigned i = 0; i < scripts.size(); ++i)
        if (scripts[i]->ShouldRun())
        {
            ParseAppAndClassName(scripts[i].get(), appName, className);
            if (appName == thisAppName)
                CreateScriptObject(app, scripts[i].get(), className);
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
    foreach(const QString &configFile, framework_->Plugins()->ConfigurationFiles())
    {
        QDomDocument doc("plugins");
        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogError("JavascriptModule::ParseStartupScriptConfig: Failed to open file \"" + configFile + "\"!");
            return QStringList();
        }
        QString errorMsg;
        if (!doc.setContent(&file, &errorMsg))
        {
            LogError("JavascriptModule::ParseStartupScriptConfig: Failed to parse XML file \"" + configFile + "\": " + errorMsg);
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
    }

    return pluginsToLoad;
}

void JavascriptModule::LoadStartupScripts()
{
    UnloadStartupScripts();

    QString path = QDir::fromNativeSeparators(Application::InstallationDirectory()) + "jsmodules/startup";
    QStringList scripts;
    foreach(const QString &file, DirectorySearch(path, false, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
    if (file.endsWith(".js", Qt::CaseInsensitive))
        scripts.append(file);

    QStringList startupScriptsToLoad = ParseStartupScriptConfig();

    // Create a script instance for each of the files, register services for it and try to run.
    LogInfo(Name() + ": Loading startup scripts from /jsmodules/startup");
    if (scripts.empty())
        LogInfo(Name() + ": ** No scripts in /jsmodules/startup");
    else
    {
        QStringList scriptsToBeRemoved;
        foreach (QString script, startupScriptsToLoad)
        {
            QString fullPath = path + "/" + script;
            if (scripts.contains(fullPath) || scripts.contains(script))
            {
                LogInfo(Name() + ": ** " + script);
                JavascriptInstance* jsInstance = new JavascriptInstance(fullPath, this);
                PrepareScriptInstance(jsInstance);
                startupScripts_.push_back(jsInstance);
                jsInstance->Run();

                scriptsToBeRemoved << fullPath;
                scriptsToBeRemoved << script;
            }
        }

        if (scriptsToBeRemoved.empty())
            LogInfo(Name() + ": ** No startup scripts were specified in config file");
        else
            foreach (QString script, scriptsToBeRemoved)
                startupScriptsToLoad.removeAll(script);
    }

    LogInfo(Name() + ": Loading scripts from startup config");
    if (startupScriptsToLoad.empty())
        LogInfo(Name() + ": ** No scripts in config");

    // Allow relative paths from '/<install_dir>' and '/<install_dir>/jsmodules'  to start also
    QDir jsPluginsDir(QDir::fromNativeSeparators(Application::InstallationDirectory()) + "jsmodules");
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
            LogWarning(Name() + "** Could not find startup file for: " + startupScript);
            continue;
        }

        LogInfo(Name() + ": ** " + startupScript);
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
    PROFILE(JSModule_PrepareScriptInstance);
    static std::set<QObject*> checked;
    
    // Register framework's dynamic properties (service objects) and the framework itself to the script engine
    QList<QByteArray> properties = framework_->dynamicPropertyNames();
    for(QList<QByteArray>::size_type i = 0; i < properties.size(); ++i)
    {
        QString name = properties[i];
        QObject* serviceobject = framework_->property(name.toStdString().c_str()).value<QObject*>();
        if (instance->RegisterService(serviceobject, name))
        {
            if (checked.find(serviceobject) == checked.end())
            {
                // Check if the service object has an OnScriptEngineCreated() slot, and give it a chance to perform further actions
                const QMetaObject* meta = serviceobject->metaObject();
                if (meta->indexOfSlot("OnScriptEngineCreated(QScriptEngine*)") != -1)
                    QObject::connect(this, SIGNAL(ScriptEngineCreated(QScriptEngine*)), serviceobject, SLOT(OnScriptEngineCreated(QScriptEngine*)));
                
                checked.insert(serviceobject);
            }
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
        instance->RegisterService(comp->ParentScene(), "scene");
    }

    emit ScriptEngineCreated(instance->Engine());
}

extern "C"
{
#ifndef ANDROID
DLLEXPORT void TundraPluginMain(Framework *fw)
#else
DEFINE_STATIC_PLUGIN_MAIN(JavascriptModule)
#endif
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new JavascriptModule();
    fw->RegisterModule(module);
}
}
