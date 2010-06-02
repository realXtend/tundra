#include "StableHeaders.h"
#include "JavascriptScriptModule.h"
#include "ModuleManager.h"
#include "ConsoleCommandServiceInterface.h"

//#include <QtScript>
#include <QScriptEngine>
#include <QtGui>
#include <QObject>
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)

//#include <QtUiTools>

//#include "QtModule.h"
//#include "UICanvas.h"

namespace JavascriptScript
{
    class JavascriptScriptModule;
}

namespace
{
    JavascriptScript::JavascriptScriptModule *javascriptScriptModuleInstance_ = 0;
}

namespace JavascriptScript
{
    JavascriptScriptModule::JavascriptScriptModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    JavascriptScriptModule::~JavascriptScriptModule()
    {
    }

    void JavascriptScriptModule::Load()
    {
      LogInfo("Module " + Name() + " loaded.");
    }

    void JavascriptScriptModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void JavascriptScriptModule::Initialize()
    {
        LogInfo("Module " + Name() + " initializing...");
		
        assert(!javascriptScriptModuleInstance_);
        javascriptScriptModuleInstance_ = this;

        // Register ourselves as javascript scripting service.
        boost::shared_ptr<JavascriptScriptModule> jsmodule = framework_->GetModuleManager()->GetModule<JavascriptScriptModule>(Foundation::Module::MT_QtScript).lock();
        boost::weak_ptr<ScriptServiceInterface> service = boost::dynamic_pointer_cast<ScriptServiceInterface>(jsmodule);
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_JavascriptScripting, service);

        QScriptValue res = engine.evaluate("1 + 1;");
        LogInfo("Javascript thinks 1 + 1 = " + res.toString().toStdString());

        engine.globalObject().setProperty("print", engine.newFunction(JavascriptScript::Print));
        //engine.globalObject().setProperty("loadUI", engine.newFunction(JavascriptScript::LoadUI));

        QScriptValue objectbutton= engine.scriptValueFromQMetaObject<QPushButton>();
	engine.globalObject().setProperty("QPushButton", objectbutton);

        engine.globalObject().setProperty("load", engine.newFunction(JavascriptScript::ScriptRunFile));

        RunString("print('Hello from qtscript');");


        QObject *x = new QObject();
        x->setObjectName("1hep");
        QObject *y = new QObject();
        y->setObjectName("2hup");
        QObject *z = new QObject();
        z->setObjectName("3hop");

        //QMap<QString, QObject*> ctx;        
        //ctx["a"] = x;
        //ctx["b"] = y;
        //ctx["c"] = z;
        //RunString("print('another hello ' + a.objectName + ' ' + b.objectName + ' ' + c.objectName)", ctx);

        QVariantMap ctx2;
        ctx2["a"] = QVariant::fromValue(x);
        ctx2["b"] = QVariant::fromValue(y);
        ctx2["c"] = QVariant::fromValue(z);
        RunString("print('another hello ' + a.objectName + ' ' + b.objectName + ' ' + c.objectName)", ctx2);

        char* js = "print('hello from ui loader & handler script!');"
                   "ui = loadUI('dialog.ui');"
                   "print(ui);"
                   "function changed(v) {"
                   "	print('val changed to: ' + v);"
                   "}"
                   "print(ui.doubleSpinBox.valueChanged);"
                   "ui.doubleSpinBox['valueChanged(double)'].connect(changed);"
                   "print('connecting to doubleSpinBox.valueChanged ok from js (?)');";
 
        //engine.evaluate(QString::fromAscii(js));
    }

    void JavascriptScriptModule::Update(f64 frametime)
    {
    }

    void JavascriptScriptModule::Uninitialize()
    {
    }

    void JavascriptScriptModule::PostInitialize()
    {
        RegisterConsoleCommand(Console::CreateCommand(
            "JsExec", "Execute given code in the embedded Javascript interpreter. Usage: JsExec(mycodestring)", 
            Console::Bind(this, &JavascriptScriptModule::ConsoleRunString))); 

        RegisterConsoleCommand(Console::CreateCommand(
            "JsLoad", "Execute a javascript file. JsLoad(myjsfile.js)", 
            Console::Bind(this, &JavascriptScriptModule::ConsoleRunFile))); 

      /* RegisterConsoleCommand(Console::CreateCommand(
            "PyReset", "Resets the Python interpreter - should free all it's memory, and clear all state.", 
            Console::Bind(this, &PythonScriptModule::ConsoleReset))); 
      */
    }

    Console::CommandResult JavascriptScriptModule::ConsoleRunString(const StringVector &params)
    {
        if (params.size() != 1)
        {            
            return Console::ResultFailure("Usage: JsExec(print 1 + 1)");
        }

        else
        {
            JavascriptScriptModule::RunString(QString::fromStdString(params[0]));
            return Console::ResultSuccess();
        }
    }

    Console::CommandResult JavascriptScriptModule::ConsoleRunFile(const StringVector &params)
    {        
        if (params.size() != 1)
        {            
            return Console::ResultFailure("Usage: JsLoad(myfile.js)");
        }

        QString scriptFileName = QString::fromStdString(params[0]);
        JavascriptScriptModule::RunScript(scriptFileName);

        return Console::ResultSuccess();
    }

  /*    void JavascriptScriptModule::RunString(QString codestr, QMap<QString, QObject*> context)
    {
        //LogInfo("Evaluating: " + scriptString.toStdString());
        QMapIterator<QString, QObject*> i(context);
        while (i.hasNext()) 
        {
            i.next();
            //LogInfo(i.key().toStdString());
            //LogInfo(i.value()->objectName().toStdString());
            engine.globalObject().setProperty(i.key(), engine.newQObject(i.value()));
        }

        engine.evaluate(codestr);
        }*/

    void JavascriptScriptModule::RunString(QString codestr, QVariantMap context)
    {
        QMapIterator<QString, QVariant> i(context);
        while (i.hasNext()) 
        {
            i.next();
            //LogInfo(i.key().toStdString());
            //LogInfo(i.value().value<QObject*>()->objectName().toStdString());
            engine.globalObject().setProperty(i.key(), engine.newQObject(i.value().value<QObject*>()));
        }

        engine.evaluate(codestr);
    }

    void JavascriptScriptModule::RunScript(QString scriptFileName)
    {
      //QFile scriptFile("jsmodules/intensity_js/intensity/" + scriptFileName);
        QFile scriptFile(scriptFileName);
        scriptFile.open(QIODevice::ReadOnly);
        engine.evaluate(scriptFile.readAll(), scriptFileName);
        scriptFile.close();
    }

    JavascriptScriptModule *JavascriptScriptModule::GetInstance()
    {
        assert(javascriptScriptModuleInstance_);
        return javascriptScriptModuleInstance_;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace JavascriptScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(JavascriptScriptModule)
POCO_END_MANIFEST

//API stuff here first

//for javascript to load a .ui file and get the widget in return, to assign connections
/*
QScriptValue JavascriptScript::LoadUI(QScriptContext *context, QScriptEngine *engine)
{
	QWidget *widget;
	QScriptValue qswidget;
    
	boost::shared_ptr<QtUI::QtModule> qt_module = JavascriptScript::staticframework->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
	boost::shared_ptr<QtUI::UICanvas> canvas_;
    
    //if ( qt_module.get() == 0)
    //    return NULL;

    canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();

    QUiLoader loader;
    QFile file("../JavascriptScriptModule/proto/dialog.ui");
    widget = loader.load(&file); 

    canvas_->AddWidget(widget);

    // Set canvas size. 
    canvas_->resize(widget->size());
	canvas_->Show();

	qswidget = engine->newQObject(widget);

	return qswidget;
        }
*/

QScriptValue JavascriptScript::Print(QScriptContext *context, QScriptEngine *engine)
{
    std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
    return QScriptValue();
}

QScriptValue JavascriptScript::ScriptRunFile(QScriptContext *context, QScriptEngine *engine)
{
    QString scriptFileName = context->argument(0).toString();

    JavascriptScriptModule *owner = JavascriptScriptModule::GetInstance();
    owner->RunScript(scriptFileName);

    return QScriptValue();
}
