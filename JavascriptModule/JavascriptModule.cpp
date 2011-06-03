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

#include "MemoryLeakCheck.h"

std::string JavascriptModule::type_name_static_ = "Javascript";
JavascriptModule *javascriptModuleInstance_ = 0;

JavascriptModule::JavascriptModule() :
    IModule(type_name_static_),
#ifndef QT_NO_SCRIPTTOOLS
	debugger(0),
	debuggerWindow(0),
	debuggerAttachedEngine(0),
	debuggerAttachedEngineEvaluating(false),
#endif
    engine(new QScriptEngine(this))
{
}

JavascriptModule::~JavascriptModule()
{
}

void JavascriptModule::Load()
{
    DECLARE_MODULE_EC(EC_Script);
    DECLARE_MODULE_EC(EC_DynamicComponent);
    framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new ScriptAssetFactory));
}

void JavascriptModule::Initialize()
{
    connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));

    LogInfo("Module " + Name() + " initializing...");

    QString pluginspath("./qtscript-plugins");
    QCoreApplication::addLibraryPath(pluginspath);
    assert(!javascriptModuleInstance_);
    javascriptModuleInstance_ = this;

    // Register ourselves as javascript scripting service.
    boost::shared_ptr<JavascriptModule> jsmodule = framework_->GetModuleManager()->GetModule<JavascriptModule>().lock();
    boost::weak_ptr<ScriptServiceInterface> service = boost::dynamic_pointer_cast<ScriptServiceInterface>(jsmodule);
    framework_->GetServiceManager()->RegisterService(Service::ST_JavascriptScripting, service);

    engine->globalObject().setProperty("print", engine->newFunction(Print));
}

void JavascriptModule::PostInitialize()
{
#ifndef QT_NO_SCRIPTTOOLS
	if (!GetFramework()->IsHeadless() && !GetFramework()->IsEditionless())
	{
		bool debugging_enable = false;

		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Javascript");
		if (settings.contains("debugging")){
			debugging_enable = settings.value("debugging", QVariant(false)).toBool();
		}

		debuggerAction = new QAction("Debugging", this);
		debuggerAction->setCheckable(true);

		GetFramework()->UiService()->AddExternalMenuAction(debuggerAction, "Debugging", "Scripts");
		connect(debuggerAction, SIGNAL(triggered(bool)), this, SLOT(OnToogleDebugging(bool)));

		if (debugging_enable)
		{
			debuggerAction->setChecked(true);

			debugger = new QScriptEngineDebugger;
			if (debugger)
			{
				connect(debugger, SIGNAL(evaluationSuspended(void)), this, SLOT(OnEvaluationSuspended(void)));
				connect(debugger, SIGNAL(evaluationResumed(void)), this, SLOT(OnEvaluationResumed(void)));
				debuggerWindow = (QWidget*) debugger->standardWindow();
				GetFramework()->UiService()->AddWidgetToScene(debuggerWindow,true,true);
				GetFramework()->UiService()->AddWidgetToMenu(debuggerWindow,"Show Debugger", "Scripts");
			}
		}
		else
		{
			debuggerAction->setChecked(false);
		}
	}
#endif

    RegisterNaaliCoreMetaTypes();
    
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
        std::vector<std::string> sv =  programOptions["run"].as<std::vector<std::string> >();
        for(std::vector<std::string>::iterator i = sv.begin(); i != sv.end(); ++i)
        {
            JavascriptInstance *jsInstance = new JavascriptInstance(i->c_str(), this);
            PrepareScriptInstance(jsInstance);
            startupScripts_.push_back(jsInstance);
            jsInstance->Run();
        }
    }
}

void JavascriptModule::Uninitialize()
{
    UnloadStartupScripts();
#ifndef QT_NO_SCRIPTTOOLS
	if (!GetFramework()->IsHeadless() && !GetFramework()->IsEditionless())
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Javascript");
		if (!debugger)
			settings.setValue("debugging", QVariant(false));
		else
		{
			settings.setValue("debugging", QVariant(true));
			if (debuggerAttachedEngine && (debuggerAttachedEngineEvaluating || debuggerAttachedEngine->isEvaluating()))
			{
				debuggerAttachedEngine->abortEvaluation();
				debuggerAttachedEngine = 0;
			}
			if (debuggerWindow)
			{
				//On exit, UiModule could have been uninitialized before.
				UiServiceInterface* uiservice = GetFramework()->UiService();
				if (uiservice)
				{
					uiservice->RemoveWidgetFromMenu(debuggerWindow);
					uiservice->RemoveWidgetFromScene(debuggerWindow);
				}
				debuggerWindow = 0;//It is removed with debugger
			}

			debugger->detach();
			debugger->deleteLater();
			debugger = 0;
		}
	}
#endif
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
    while (i.hasNext())
    {
        i.next();
        engine->globalObject().setProperty(i.key(), engine->newQObject(i.value().value<QObject*>()));
    }
#ifndef QT_NO_SCRIPTTOOLS
	bool attachedToDebugger = attachToDebugger(engine);
	if(attachedToDebugger)
	{
		setAttachedEngineEvaluating(true);
	}
#endif

	engine->evaluate(codestr);

#ifndef QT_NO_SCRIPTTOOLS
	if(attachedToDebugger)
		setAttachedEngineEvaluating(false);
#endif
    
}

void JavascriptModule::RunScript(const QString &scriptFileName)
{
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
#ifndef QT_NO_SCRIPTTOOLS
	bool attachedToDebugger = attachToDebugger(engine);
	if(attachedToDebugger)
	{
		setAttachedEngineEvaluating(true);
	}
#endif

	engine->evaluate(scriptFile.readAll(), scriptFileName);

#ifndef QT_NO_SCRIPTTOOLS
	if(attachedToDebugger)
		setAttachedEngineEvaluating(false);
#endif
    
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
    
    std::string path = "./jsmodules/startup";
    std::vector<std::string> scripts;

    try
    {
        boost::filesystem::directory_iterator i(path);
        boost::filesystem::directory_iterator end_iter;
        while (i != end_iter)
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
    catch (std::exception &/*e*/)
    {
    }
    
    // Create a script instance for each of the files, register services for it and try to run.
    for (uint i = 0; i < scripts.size(); ++i)
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
    for (uint i = 0; i < startupScripts_.size(); ++i)
        startupScripts_[i]->Unload();
    for (uint i = 0; i < startupScripts_.size(); ++i)
        delete startupScripts_[i];
    startupScripts_.clear();
}

void JavascriptModule::PrepareScriptInstance(JavascriptInstance* instance, EC_Script *comp)
{
    static std::set<QObject*> checked;
    
    // Register framework's dynamic properties (service objects) and the framework itself to the script engine
    QList<QByteArray> properties = framework_->dynamicPropertyNames();
    for (QList<QByteArray>::size_type i = 0; i < properties.size(); ++i)
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
    
    for(int i = 0; i < properties.size(); ++i)
        instance->RegisterService(framework_->property(properties[i]).value<QObject*>(), properties[i]);

    if (comp)
    {
        // Set entity and scene that own the EC_Script component.
        instance->RegisterService(comp->GetParentEntity(), "me");
        instance->RegisterService(comp->GetParentEntity()->GetScene(), "scene");
    }

    emit ScriptEngineCreated(instance->GetEngine());
}
#ifndef QT_NO_SCRIPTTOOLS
void JavascriptModule::OnToogleDebugging(bool checked)
{
	if (!checked || debugger)
	{
		if (debuggerAttachedEngine && debuggerAttachedEngine->isEvaluating())
		{
			debuggerAction->setChecked(true);
			return;
		}

		if (debuggerWindow)
		{
			GetFramework()->UiService()->RemoveWidgetFromMenu(debuggerWindow);
			GetFramework()->UiService()->RemoveWidgetFromScene(debuggerWindow);
			debuggerWindow = 0;//It is removed with debugger
		}
		if(debugger)
		{
			debugger->detach();
			debugger->deleteLater();
			debugger = 0;
		}
	}
	if (checked)
	{
		debugger = new QScriptEngineDebugger;
		if (debugger)
		{
			connect(debugger, SIGNAL(evaluationSuspended(void)), this, SLOT(OnEvaluationSuspended(void)));
			debuggerWindow = (QWidget*) debugger->standardWindow();
			GetFramework()->UiService()->AddWidgetToScene(debuggerWindow,true,true);
			GetFramework()->UiService()->AddWidgetToMenu(debuggerWindow,"Show Debugger", "Scripts");
		}
	}

}

void JavascriptModule::setAttachedEngineEvaluating(bool evaluating)
{
	debuggerAttachedEngineEvaluating = evaluating;
	if (evaluating)
	{
		if (debuggerAction)
			debuggerAction->setEnabled(false);
	}
	else
	{
		if (debuggerAction && debugger && debugger->state() == QScriptEngineDebugger::RunningState)
			debuggerAction->setEnabled(true);
	}
}

void JavascriptModule::OnEvaluationSuspended(void)
{
	if (debuggerAction)
		debuggerAction->setEnabled(false);

	if (debuggerWindow)
	{
		debuggerWindow->setEnabled(true);
		GetFramework()->UiService()->ShowWidget(debuggerWindow);
	}
}

void JavascriptModule::OnEvaluationResumed(void)
{
	//Wait for enabling debuggerAction until the scriptengine has been detached
	if (debuggerAction && !debuggerAttachedEngineEvaluating)
		debuggerAction->setEnabled(true);

	//Disable automatic hiding of debugger
	//if (debuggerWindow)
	//	GetFramework()->UiService()->HideWidget(debuggerWindow);
}

bool JavascriptModule::attachToDebugger(QScriptEngine* scriptEngine)
{
	if(!debuggerAttachedEngineEvaluating && debugger)
	{
		debugger->detach();
		debugger->attachTo(scriptEngine);
		debuggerAttachedEngine = scriptEngine;
		return true;
	}
	return false;
}

void JavascriptModule::detachFromDebugger(QScriptEngine* scriptEngine)
{
	if (debuggerAttachedEngine == scriptEngine)
	{
		debugger->detach();
		debuggerAttachedEngine = 0;
	}
}
#endif



QScriptValue Print(QScriptContext *context, QScriptEngine *engine)
{
    std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
    return QScriptValue();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(JavascriptModule)
POCO_END_MANIFEST
