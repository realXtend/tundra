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

#include "Application.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "AssetAPI.h"
#include "EC_Script.h"
#include "ScriptAssetFactory.h"
#include "EC_DynamicComponent.h"
#include "SceneManager.h"
#include "InputAPI.h"
#include "UiServiceInterface.h"
#include "AudioAPI.h"
#include "FrameAPI.h"
#include "ConsoleAPI.h"
#include "ConsoleCommandUtils.h"

#include "ScriptAsset.h"

#include <QtScript>

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
    DECLARE_MODULE_EC(EC_Script);
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
    
    framework_->Console()->RegisterCommand(CreateConsoleCommand(
        "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)", 
        ConsoleBind(this, &JavascriptModule::ConsoleRunString)));

    framework_->Console()->RegisterCommand(CreateConsoleCommand(
        "JsLoad", "Execute a javascript file. JsLoad(myjsfile.js)",
        ConsoleBind(this, &JavascriptModule::ConsoleRunFile)));
    
    framework_->Console()->RegisterCommand(CreateConsoleCommand(
        "JsReloadScripts", "Reloads and re-executes startup scripts.",
        ConsoleBind(this, &JavascriptModule::ConsoleReloadScripts)));
    
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
    RESETPROFILER;
}

ConsoleCommandResult JavascriptModule::ConsoleRunString(const StringVector &params)
{
    if (params.size() != 1)
        return ConsoleResultFailure("Usage: JsExec(print 1 + 1)");

    JavascriptModule::RunString(QString::fromStdString(params[0]));
    return ConsoleResultSuccess();
}

ConsoleCommandResult JavascriptModule::ConsoleRunFile(const StringVector &params)
{
    if (params.size() != 1)
        return ConsoleResultFailure("Usage: JsLoad(myfile.js)");

    JavascriptModule::RunScript(QString::fromStdString(params[0]));

    return ConsoleResultSuccess();
}

ConsoleCommandResult JavascriptModule::ConsoleReloadScripts(const StringVector &params)
{
    LoadStartupScripts();

    return ConsoleResultSuccess();
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
    Scene::ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    connect(scene.get(), SIGNAL(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)));
    connect(scene.get(), SIGNAL(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)),
            SLOT(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)));
}

void JavascriptModule::ScriptAssetChanged(ScriptAssetPtr newScript)
{
    PROFILE(JSModule_ScriptAssetChanged);

    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    assert(sender && "JavascriptModule::ScriptAssetChanged needs to be invoked from EC_Script!");
    if (!sender)
        return;

    // First clean up any previous running script from EC_Script, if any exists.
    // (but only clean up scripts of our script type, other engines can clean up theirs)
    if (dynamic_cast<JavascriptInstance*>(sender->GetScriptInstance()))
        sender->SetScriptInstance(0);

    // EC_Script can host scripts of different types, and all script engines listen to asset changes.
    // First we'll need to validate whether the user even specified a script file that's QtScript.
    QString scriptType = sender->type.Get().trimmed().toLower();
    if (scriptType != "js" && scriptType.length() > 0)
        return; // The user enforced a foreign script type using the EC_Script type field.

    if (newScript->Name().endsWith(".js") || scriptType == "js") // We're positively using QtScript.
    {
        JavascriptInstance *jsInstance = new JavascriptInstance(newScript, this);
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
            sender->Run();
    }
}

void JavascriptModule::ComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
        connect(comp, SIGNAL(ScriptAssetChanged(ScriptAssetPtr)), this, SLOT(ScriptAssetChanged(ScriptAssetPtr)), Qt::UniqueConnection);
}

void JavascriptModule::ComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (comp->TypeName() == EC_Script::TypeNameStatic())
        disconnect(comp, SIGNAL(ScriptAssetChanged(ScriptAssetPtr)), this, SLOT(ScriptAssetChanged(ScriptAssetPtr)));
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
    
    // Create a script instance for each of the files, register services for it and try to run.
    for(uint i = 0; i < scripts.size(); ++i)
    {
        JavascriptInstance* jsInstance = new JavascriptInstance(QString::fromStdString(scripts[i]), this);
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
    
    for(uint i = 0; i < properties.size(); ++i)
        instance->RegisterService(framework_->property(properties[i]).value<QObject*>(), properties[i]);

    if (comp)
    {
        // Set entity and scene that own the EC_Script component.
        instance->RegisterService(comp->GetParentEntity(), "me");
        instance->RegisterService(comp->GetParentEntity()->GetScene(), "scene");
    }

    emit ScriptEngineCreated(instance->GetEngine());
}

QScriptValue Print(QScriptContext *context, QScriptEngine *engine)
{
    std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
    return QScriptValue();
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new JavascriptModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
