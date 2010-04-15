#include "StableHeaders.h"
#include "QtScriptModule.h"
#include <QtScript>

//#include <QtUiTools>

//#include "QtModule.h"
//#include "UICanvas.h"

namespace RexQtScript
{
	RexQtScriptModule::RexQtScriptModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    RexQtScriptModule::~RexQtScriptModule()
    {
    }

	void RexQtScriptModule::Load()
	{
        LogInfo("Module " + Name() + " loaded.");
	}

    void RexQtScriptModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void RexQtScriptModule::Initialize()
    {
        LogInfo("Module " + Name() + " initializing...");
		
        //XXX hack to have a ref to framework for api funcs
        RexQtScript::staticframework = framework_;

        QScriptValue res = engine.evaluate("1 + 1;");
        LogInfo("Javascript thinks 1 + 1 = " + res.toString().toStdString());

        engine.globalObject().setProperty("print", engine.newFunction(RexQtScript::Print));
        //engine.globalObject().setProperty("loadUI", engine.newFunction(RexQtScript::LoadUI));

        engine.evaluate("print('Hello from qtscript');");

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

    void RexQtScriptModule::Update(f64 frametime)
    {
    }

    void RexQtScriptModule::Uninitialize()
    {
    }

    void RexQtScriptModule::PostInitialize()
    {
    }

    /*QScriptValue RexQtScriptModule::test(QScriptContext *context, QScriptEngine *engine)
    {
	}*/

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace RexQtScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(RexQtScriptModule)
POCO_END_MANIFEST

//API stuff here first

//for javascript to load a .ui file and get the widget in return, to assign connections
/*
QScriptValue RexQtScript::LoadUI(QScriptContext *context, QScriptEngine *engine)
{
	QWidget *widget;
	QScriptValue qswidget;
    
	boost::shared_ptr<QtUI::QtModule> qt_module = RexQtScript::staticframework->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
	boost::shared_ptr<QtUI::UICanvas> canvas_;
    
    //if ( qt_module.get() == 0)
    //    return NULL;

    canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();

    QUiLoader loader;
    QFile file("../RexQtScriptModule/proto/dialog.ui");
    widget = loader.load(&file); 

    canvas_->AddWidget(widget);

    // Set canvas size. 
    canvas_->resize(widget->size());
	canvas_->Show();

	qswidget = engine->newQObject(widget);

	return qswidget;
        }*/

QScriptValue RexQtScript::Print(QScriptContext *context, QScriptEngine *engine)
{
	std::cout << "{QtScript} " << context->argument(0).toString().toStdString() << "\n";
	return QScriptValue();
}
