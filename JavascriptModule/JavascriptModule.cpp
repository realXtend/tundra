/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "JavascriptModule.h"
#include "EC_Script.h"
#include "SceneManager.h"
#include "InputContext.h"
#include "InputServiceInterface.h"
#include "UiServiceInterface.h"
#include "ScriptMetaTypeDefines.h"
#include "JavascriptEngine.h"

#include <QtScript>

#include "ConsoleCommandServiceInterface.h"
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)

#include "MemoryLeakCheck.h"

std::string JavascriptModule::type_name_static_ = "JavascriptModule";
JavascriptModule *javascriptModuleInstance_ = 0;

JavascriptModule::JavascriptModule() :
    ModuleInterface(type_name_static_)
{
}

JavascriptModule::~JavascriptModule()
{
}

void JavascriptModule::Load()
{
    DECLARE_MODULE_EC(EC_Script);
}


void JavascriptModule::PreInitialize()
{
}

void JavascriptModule::Initialize()
{
    connect(GetFramework(), SIGNAL(SceneAdded(const QString&)), this, SLOT(SceneAdded(const QString&)));

    LogInfo("Module " + Name() + " initializing...");

    assert(!javascriptModuleInstance_);
    javascriptModuleInstance_ = this;

    // Register ourselves as javascript scripting service.
    boost::shared_ptr<JavascriptModule> jsmodule = framework_->GetModuleManager()->GetModule<JavascriptModule>().lock();
    boost::weak_ptr<ScriptServiceInterface> service = boost::dynamic_pointer_cast<ScriptServiceInterface>(jsmodule);
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_JavascriptScripting, service);

    engine.globalObject().setProperty("print", engine.newFunction(Print));

    QScriptValue objectbutton= engine.scriptValueFromQMetaObject<QPushButton>();
    engine.globalObject().setProperty("QPushButton", objectbutton);

    JavascriptModule::RunScript("jsmodules/lib/json2.js");

    RunString("print('Hello from qtscript');");
}

void JavascriptModule::PostInitialize()
{
    input_ = GetFramework()->Input().RegisterInputContext("ScriptInput", 100);
    Foundation::UiServiceInterface *ui = GetFramework()->GetService<Foundation::UiServiceInterface>();

    services_["Input"]  = input_.get();
    services_["Ui"]     = ui;

    RegisterConsoleCommand(Console::CreateCommand(
        "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)", 
        Console::Bind(this, &JavascriptModule::ConsoleRunString))); 

    RegisterConsoleCommand(Console::CreateCommand(
        "JsLoad", "Execute a javascript file. JsLoad(myjsfile.js)", 
        Console::Bind(this, &JavascriptModule::ConsoleRunFile))); 
}

Console::CommandResult JavascriptModule::ConsoleRunString(const StringVector &params)
{
    if (params.size() != 1)
    {            
        return Console::ResultFailure("Usage: JsExec(print 1 + 1)");
    }

    else
    {
        JavascriptModule::RunString(QString::fromStdString(params[0]));
        return Console::ResultSuccess();
    }
}

Console::CommandResult JavascriptModule::ConsoleRunFile(const StringVector &params)
{        
    if (params.size() != 1)
    {            
        return Console::ResultFailure("Usage: JsLoad(myfile.js)");
    }

    QString scriptFileName = QString::fromStdString(params[0]);
    JavascriptModule::RunScript(scriptFileName);

    return Console::ResultSuccess();
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
        engine.globalObject().setProperty(i.key(), engine.newQObject(i.value().value<QObject*>()));
    }

    engine.evaluate(codestr);
}

void JavascriptModule::RunScript(const QString &scriptFileName)
{
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
    engine.evaluate(scriptFile.readAll(), scriptFileName);
    scriptFile.close();
}

void JavascriptModule::Uninitialize()
{
}

void JavascriptModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool JavascriptModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

void JavascriptModule::SceneAdded(const QString &name)
{
    Scene::ScenePtr scene = GetFramework()->GetScene(name.toStdString());
    connect(scene.get(), SIGNAL(ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)),
            this, SLOT(ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)));
    connect(scene.get(), SIGNAL(ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)),
            this, SLOT(ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type)));
    services_["Scene"]  = scene.get();
}

void JavascriptModule::ScriptChanged(const QString &scriptRef)
{
    EC_Script *sender = dynamic_cast<EC_Script*>(this->sender());
    if(!sender && sender->type.Get()!= "js") 
        return;

    JavascriptEngine *javaScriptInstance = new JavascriptEngine(scriptRef);
    sender->SetScriptInstance(javaScriptInstance);

    //Register all services to script engine.
    ServiceMap::iterator iter = services_.begin();
    for(; iter != services_.end(); iter++)
        javaScriptInstance->RegisterService(iter.value(), iter.key());
    sender->Run();
}

void JavascriptModule::ComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
{
    if(comp->TypeName() == "EC_Script")
        connect(comp, SIGNAL(ScriptRefChanged(const QString &)),
                this, SLOT(ScriptChanged(const QString &)));
}

void JavascriptModule::ComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
{
    if(comp->TypeName() == "EC_Script")
        disconnect(comp, SIGNAL(ScriptRefChanged(const QString &)),
                   this, SLOT(ScriptChanged(const QString &)));
}

QScriptValue Print(QScriptContext *context, QScriptEngine *engine)
{
    std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
    return QScriptValue();
}

QScriptValue ScriptRunFile(QScriptContext *context, QScriptEngine *engine)
{
    QString scriptFileName = context->argument(0).toString();

    JavascriptModule *owner = JavascriptModule::GetInstance();
    owner->RunScript(scriptFileName);

    return QScriptValue();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(JavascriptModule)
POCO_END_MANIFEST
