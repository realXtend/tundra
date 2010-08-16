// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
//#include <Python.h> //to add CreateCanvas first as a normal py func
#include "RexPythonQt.h"
#include <PythonQt.h>
#include <PythonQt_QtAll.h>
//#include "UiWidgetProperties.h"
//#include "UiProxyWidget.h"

//#include <QGroupBox> //just for testing addObject
//#include <QUiLoader> //for .ui loading in testing

//#include "QtModule.h"
//#include "UICanvas.h"
//#include "PythonScriptModule.h"
//#include "gui/PythonQtScriptingConsole.h"

#include "MemoryLeakCheck.h"

namespace PythonScript
{
    void initRexQtPy(PyObject* apimodule)
    {
        //XXX add shutdown too!
        // init PythonQt, but not Python 'cause PythonScriptModule has already done that.
        PythonQt::init(PythonQt::PythonAlreadyInitialized);
        PythonQt_QtAll::init();
        //PythonQt::self()->registerClass(&UiProxyWidget::staticMetaObject);
//        PythonQt::self()->registerClass(&UiServices::UiWidgetProperties::staticMetaObject);
        // get the __main__ python module
        //PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
  
        // evaluate a simple python script and receive the result a qvariant:
        //QVariant result = mainModule.evalScript("19*2+4", Py_eval_input);

        // add the canvas creation func from here to the rexviewer py api
        // now in PythonScriptModule.cpp due to staticframework ref prob
        /*
        PyObject *moduleDict = PyModule_GetDict(apimodule);
        static PyMethodDef createcanvasdef[] = {{"createCanvas", CreateCanvas, METH_VARARGS, "Create a new Qt canvas within the viewer"}};
        PyObject *createCanvasFunc = PyCFunction_New(createcanvasdef, NULL);
        PyDict_SetItemString(moduleDict, "createCanvas", createCanvasFunc); 
        */
    }

    // the whole thing to PythonScriptModule.cpp due to the staticframework prob..
    /*
    PyObject* CreateCanvas(PyObject *self, PyObject *args)
    {        
        PythonScript::self()->LogInfo("Creating canvas in RexPythonQt!");
        if (!PythonScript::self()->GetFramework()) //<- this bit is null always... and crashes the viewer aswell... what gives
        {
            //std::cout << "Oh crap staticframework is not there!";
            PythonScript::self()->LogInfo("PythonScript's framework is not present!");
            return NULL;
        }

        boost::shared_ptr<QtUI::QtModule> qt_module = PythonScript::self()->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        boost::shared_ptr<QtUI::UICanvas> canvas_;
        
        if ( qt_module.get() == 0)
            return NULL;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();

        //QtUI::UICanvas* qcanvas = canvas_.get();
            
        //these can be done on the py side too, so decoupled from this:
        QWidget *widget;
        QUiLoader loader;
        QFile file("pymodules/editgui/editobject.ui");
        widget = loader.load(&file); 
        canvas_->AddWidget(widget);
        canvas_->Show();

        return PythonScriptModule::GetInstance()->WrapQObject(widget); //box); //qcanvas
    }
    //*/
    /*PyObject* CreateCanvas(PyObject *self, PyObject *args)
    {        
        mod = PythonScript::self();
        mod->LogInfo
        frm = mod->GetFrae
        if (!PythonScript::staticframework)
        {
            std::cout << "Oh crap staticframework is not there!";
            Py_RETURN_NONE;
        }

        boost::shared_ptr<QtUI::QtModule> qt_module = PythonScript::staticframework->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        boost::shared_ptr<QtUI::UICanvas> canvas_;
        
        //if ( qt_module.get() == 0)
        //    return NULL;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();
    

        //QtUI::UICanvas* qcanvas = canvas_.get();

        //a hack 'cause don't know how to easily make a func that'd return a pythonqt object
        //a proper solution is to make this a factory method in a wrapper class, http://pythonqt.sourceforge.net/index.html#Decorators
        //QString* qtest = new QString("test string");
        /*PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
        QGroupBox* qtest = new QGroupBox;
        mainModule.addObject("__test__", qtest);*/
        //QtUI::UICanvas* qcanvas = canvas_.get();
        //mainModule.addObject("__UIcanvas__", qcanvas);
        
        //Py_RETURN_NONE;

        /* these can be done on the py side too, so decoupled from this:
        QUiLoader loader;
        QFile file("../RexQtScriptModule/proto/dialog.ui");
        widget = loader.load(&file); 

        canvas_->AddWidget(widget);
    }
    */



            /*
        // Create object from python, hold onto reference in C++:
        PythonQtObjectPtr tag = mainModule.evalScript("EyeD3Tagger()\n", Py_eval_input);
        Q_ASSERT(!tag.isNull());

        // call python methods from C++
        tag.call("setFileName", QVariantList() << "t.mp3");
        QVariant fn = tag.call("fileName", QVariantList());
        Q_ASSERT(fn.toString() == QString("t.mp3"));

        // create a small Qt GUI
        QVBoxLayout*  vbox = new QVBoxLayout;
        QGroupBox*    box  = new QGroupBox;
        QTextBrowser* browser = new QTextBrowser(box);
        QLineEdit*    edit = new QLineEdit(box);
        QPushButton*  button = new QPushButton(box);
        button->setObjectName("button1");
        edit->setObjectName("edit");
        browser->setObjectName("browser");
        vbox->addWidget(browser);
        vbox->addWidget(edit);
        vbox->addWidget(button);
        box->setLayout(vbox);
  
        // make the groupbox to the python under the name "box"
        mainModule.addObject("box", box);
            
        // evaluate the python script which is defined in the resources
        mainModule.evalFile(":GettingStarted.py");
          
        // define a python method that appends the passed text to the browser
        mainModule.evalScript("def appendText(text):\n  box.browser.append(text)");
        // shows how to call the method with a text that will be append to the browser
        mainModule.call("appendText", QVariantList() << "The ultimate answer is ");
        */


}
