// For conditions of distribution and use, see copyright notice in license.txt

// POTENTIALLY OLD COMMENTS/INFO MOVED AT THE END OF THIS FILE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

// PythonScriptModule
#include "PythonScriptModule.h"
#include "LoggingFunctions.h"

// Framework and APIs
#include "Framework.h"
#include "CoreTypes.h"
#include "CoreDefines.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "InputAPI.h"
#include "FrameAPI.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "AudioAPI.h"
#include "UiAPI.h"

/// Framework, SceneAPI and Kristalli
#include "Application.h"
#include "IRenderer.h"
#include "Scene.h"
#include "Entity.h"
#include "IComponent.h"
#include "IComponentFactory.h"
#include "IAttribute.h"
#include "UserConnection.h"

// InputAPI
#include "InputContext.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "GestureEvent.h"

// UiAPI
#include "UiGraphicsView.h"
#include "UiMainWindow.h"
#include "UiProxyWidget.h"

// Asset related
#include "AssetReference.h"
#include "IAssetTypeFactory.h"
#include "ScriptAsset.h"
#include "ScriptAssetFactory.h"
#include "AssetReferenceDecorator.h"

// Python and PythonQt
#include <PythonQt.h>
#include <PythonQt_QtAll.h>
#include "PythonScriptInstance.h"
#include "PythonEngine.h"
#include "PythonQtScriptingConsole.h"
#include "TundraWrapper.h"

// OgreRenderingModule
#include "OgreRenderingModule.h"
#include "Renderer.h"

// ECs from various places
#include "EC_Mesh.h"
#include "EC_Placeable.h"
//#include "EC_WebView.h"         // Remove to clear dependency to declaring plugin?
//#include "EC_WidgetCanvas.h"    // Remove to clear dependency to declaring plugin?
#include "EC_Camera.h"
#ifdef EC_Touchable_ENABLED
#include "EC_Touchable.h"
#endif
#include "EC_DynamicComponent.h"

// EC_Script and its Asset
#include "EC_Script.h"
#include "ScriptAsset.h"

#include "MemoryLeakCheck.h" // Keep this as the last include

namespace PythonScript
{
    //////////////// PythonScriptModule

    // Static
    /// \todo Are these really needed with the PythonQt integration?

    PythonScriptModule *PythonScriptModule::pythonScriptModuleInstance_ = 0;
    PythonScriptModule* PythonScriptModule::GetInstance() { return pythonScriptModuleInstance_; }

    // Public

    PythonScriptModule::PythonScriptModule() :
        IModule("PythonScriptModule"),
        pmmInstance(0),
        pmmModule(0),
        pmmClass(0),
        pmmDict(0),
        pythonQtInitialized_(false)
    {
    }

    PythonScriptModule::~PythonScriptModule()
    {
        pythonScriptModuleInstance_ = 0;
    }

    PyObject *PythonScriptModule::WrapQObject(QObject* qobj) const
    {
        return PythonQt::self()->priv()->wrapQObject(qobj);
    }

    //////////////// IModule overrides

    void PythonScriptModule::Load()
    {
        /// \todo This should print a error as PythonScriptModule::Load might have done this already! How to avoid?
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Script>));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new ScriptAssetFactory()));
    }

    void PythonScriptModule::Unload()
    {
        pythonScriptModuleInstance_ = 0;
        input.reset();

        pmmInstance = 0;
        pmmModule = 0;
        pmmClass = 0;
        pmmDict = 0;
    }

    void PythonScriptModule::Initialize()
    {
        // Init Python engine
        if (!engine_)
            engine_ = PythonScript::PythonEnginePtr(new PythonScript::PythonEngine(framework_));
        engine_->Initialize();

        // Set static self
        pythonScriptModuleInstance_ = this;

        if (!pythonQtInitialized_)
        {
            // Init PythonQt, implemented in RexPythonQt.cpp
            PythonQt::init(PythonQt::PythonAlreadyInitialized);
            PythonQt_QtAll::init();

            PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
            if (mainModule.isNull())
            {
                LogError("PythonScriptModule::Initialize(): Failed to get main module from PythonQt after init!");
                return;
            }

            // Add PythonScriptModule as '_pythonscriptmodule' 
            // and '_tundra' as the framework to python
            mainModule.addObject("_pythonscriptmodule", this);
            mainModule.addObject("_tundra", GetFramework());

            /////////////////////////////////////////////////////////////////////////////////////////
            // Framework and APIs
            PythonQt::self()->registerClass(&Framework::staticMetaObject);
            PythonQt::self()->registerClass(&SceneAPI::staticMetaObject);
            PythonQt::self()->registerClass(&AssetAPI::staticMetaObject);
            PythonQt::self()->registerClass(&InputAPI::staticMetaObject);
            PythonQt::self()->registerClass(&FrameAPI::staticMetaObject);
            PythonQt::self()->registerClass(&ConsoleAPI::staticMetaObject);
            PythonQt::self()->registerClass(&ConfigAPI::staticMetaObject);
            PythonQt::self()->registerClass(&AudioAPI::staticMetaObject);
            PythonQt::self()->registerClass(&UiAPI::staticMetaObject);

            /////////////////////////////////////////////////////////////////////////////////////////
            // ECs
            /// \todo Is this manual EC declaring needed anymore?
            PythonQt::self()->registerClass(&EC_Camera::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Mesh::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Placeable::staticMetaObject);
            //PythonQt::self()->registerClass(&EC_WidgetCanvas::staticMetaObject);    // Remove to clear dependency to declaring plugin?
            //PythonQt::self()->registerClass(&EC_WebView::staticMetaObject);         // Remove to clear dependency to declaring plugin?
            // Conditional ECs
#ifdef EC_Touchable_ENABLED
            PythonQt::self()->registerClass(&EC_Touchable::staticMetaObject);
#endif

            /////////////////////////////////////////////////////////////////////////////////////////
            // Scene
            PythonQt::self()->registerClass(&Scene::staticMetaObject);
            PythonQt::self()->registerClass(&Entity::staticMetaObject);
            PythonQt::self()->registerClass(&IComponent::staticMetaObject);
            PythonQt::self()->registerClass(&EntityAction::staticMetaObject);
            PythonQt::self()->registerClass(&AttributeChange::staticMetaObject);
            //PythonQt::self()->registerClass(&IAttribute::staticMetaObject); // not a qobject

            /////////////////////////////////////////////////////////////////////////////////////////
            // Rendering
            PythonQt::self()->registerClass(&OgreRenderer::OgreRenderingModule::staticMetaObject);
            PythonQt::self()->registerClass(&OgreRenderer::Renderer::staticMetaObject);

            /////////////////////////////////////////////////////////////////////////////////////////
            // Ui
            PythonQt::self()->registerClass(&UiMainWindow::staticMetaObject);
            PythonQt::self()->registerClass(&UiGraphicsView::staticMetaObject);
            PythonQt::self()->registerClass(&UiProxyWidget::staticMetaObject);

            /////////////////////////////////////////////////////////////////////////////////////////
            // Input
            PythonQt::self()->registerClass(&InputContext::staticMetaObject);
            PythonQt::self()->registerClass(&KeyEvent::staticMetaObject);
            PythonQt::self()->registerClass(&MouseEvent::staticMetaObject);
            PythonQt::self()->registerClass(&GestureEvent::staticMetaObject);

            /////////////////////////////////////////////////////////////////////////////////////////
            // Kristalli / kNet
            PythonQt::self()->registerClass(&UserConnection::staticMetaObject);

            /////////////////////////////////////////////////////////////////////////////////////////
            // Misc
            PythonQt::self()->registerClass(&ConsoleCommand::staticMetaObject);
            PythonQt::self()->registerClass(&DelayedSignal::staticMetaObject);
            PythonQt::self()->registerClass(&PythonQtScriptingConsole::staticMetaObject); // ??

            /////////////////////////////////////////////////////////////////////////////////////////
            // Decorators
            /// \todo Reimplemented decorators for new math lib
            PythonQt::self()->addInstanceDecorators(new TundraInstanceDecorator());
            PythonQt::self()->addDecorators(new TundraDecorator());
            PythonQt::self()->registerCPPClass("AssetReference");
        }

        LogInfo(Name() + ": Initialized Python and PythonQt successfully");

        // Load the Python written module manager using the Python C API directly
        pmmModule = PyImport_ImportModule("modulemanager");
        if (pmmModule == NULL) 
        {
            LogError("PythonScriptModule::Initialize(): Failed to import py modulemanager");
            return;
        }
        pmmDict = PyModule_GetDict(pmmModule);
        if (pmmDict == NULL) 
        {
            LogError("PythonScriptModule::Initialize(): Unable to get modulemanager module namespace");
            return;
        }
        pmmClass = PyDict_GetItemString(pmmDict, "ModuleManager");
        if (pmmClass == NULL) 
        {
            LogError("PythonScriptModule::Initialize(): Unable get ModuleManager class from modulemanager namespace");
            return;
        }

        LogInfo(Name() + ": Aqcuired Python ModuleManager successfully");
    }

    void PythonScriptModule::PostInitialize()
    {
        if (!pmmClass)
        {
            LogError("PythonScriptModule::Initialize() was not successful in fetching Python ModuleManager, aborting PostInitialize!");
            return;
        }

        // Now that the event constants etc are there, can instanciate the manager which triggers the loading of components
        /// \todo Remove?
        if (PyCallable_Check(pmmClass)) 
        {
            pmmInstance = PyObject_CallObject(pmmClass, NULL); 
            LogInfo(Name() + ": Initialized Python ModuleManager successfully");
        } 
        else 
            LogInfo(Name() + ": Unable to Initialized Python ModuleManager");

        // Connect to SceneAPI
        QObject::connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString&)));

        // Connect to FrameAPI
        QObject::connect(GetFramework()->Frame(), SIGNAL(Updated(float)), this, SLOT(UpdatePython(float)));

        // Console commands
        GetFramework()->Console()->RegisterCommand("PyExec", "Execute given code in the embedded Python interpreter. Usage: PyExec(mycodestring)", 
                                                   this, SLOT(ConsoleRunString(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyLoad", "Execute a python file. Usage: PyLoad(mypymodule)", 
                                                   this, SLOT(ConsoleRunFile(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyReset", "Resets the Python interpreter - should free all it's memory, and clear all state.", 
                                                   this, SLOT(ConsoleReset(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyReset", "Resets the Python interpreter - should free all it's memory, and clear all state.", 
                                                   this, SLOT(ShowConsole()));
        
        // Get framework program options and see if user wanted to launch a python script on startup.
        if (engine_)
        {
            const boost::program_options::variables_map &programOptions = GetFramework()->ProgramOptions();
            if (programOptions.count("python"))
                engine_->RunScript(programOptions["python"].as<std::string>().c_str());
            if (programOptions.count("p"))
                engine_->RunScript(programOptions["p"].as<std::string>().c_str());   
            return; 
        }
        else
            LogError("PythonScriptModule::ProcessCommandLineOptions(): Python engine is null!");        
    }

    void PythonScriptModule::Uninitialize()
    {
        // Make python exit
        if (pmmInstance)
            PyObject_CallMethod(pmmInstance, "exit", "");

        // Uninitialize python engine
        engine_->Uninitialize();
        engine_.reset();

        // Clear created input context shared ptrs, this will remove
        // the ref count and they should be deleted.
        createdInputs_.clear();        
    }
    
    //////////////// PythonScriptModule private slots

    void PythonScriptModule::OnSceneAdded(const QString &name)
    {
        Scene *scene = GetFramework()->Scene()->GetScene(name).get();
        if (scene)
        {
            QObject::connect(scene, SIGNAL(ComponentAdded(Entity*, IComponent*, AttributeChange::Type)),
                             this, SLOT(OnComponentAdded(Entity*, IComponent*)));
            QObject::connect(scene, SIGNAL(ComponentRemoved(Entity*, IComponent*, AttributeChange::Type)),
                             this, SLOT(OnComponentRemoved(Entity*, IComponent*)));
        }
    }

    void PythonScriptModule::OnComponentAdded(Entity *entity, IComponent *component)
    {
        if (component->TypeName() == EC_Script::TypeNameStatic())
            QObject::connect(component, SIGNAL(ScriptAssetChanged(const std::vector<ScriptAssetPtr>&)), 
                             this, SLOT(LoadScript(const std::vector<ScriptAssetPtr>&)), Qt::UniqueConnection);
    }

    void PythonScriptModule::OnComponentRemoved(Entity *entity, IComponent *component)
    {
        if (component->TypeName() == EC_Script::TypeNameStatic())
            QObject::disconnect(component, SIGNAL(ScriptAssetChanged(ScriptAssetPtr)), this, SLOT(LoadScript(ScriptAssetPtr)));
    }

    //////////////// PythonScriptModule public slots

    void PythonScriptModule::LoadScript(const std::vector<ScriptAssetPtr> &newScripts)
    {
        EC_Script *script = dynamic_cast<EC_Script*>(this->sender());
        if (!script)
            return;

        std::vector<ScriptAssetPtr>::const_iterator iter = newScripts.begin();
        while (iter != newScripts.end())
        {
            ScriptAssetPtr ptr = (*iter);
            const QString scriptAssetRef = ptr->Name();
            if (scriptAssetRef.endsWith(".py"))
            {
                PythonScriptInstance *pyInstance = new PythonScriptInstance(scriptAssetRef, script->ParentEntity());
                script->SetScriptInstance(pyInstance);
                if (script->getrunOnLoad())
                    script->Run();
            }
            ++iter;
        }
    }

    void PythonScriptModule::UpdatePython(float frametime)
    {
        /// \bug Somehow this causes extreme lag in a mode without console (?)
        if (pmmInstance)
            PyObject_CallMethod(pmmInstance, "run", "f", frametime);
    }

    OgreRenderer::Renderer* PythonScriptModule::GetRenderer() const
    {
        OgreRenderer::OgreRenderingModule *ogreModule = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>();
        if (ogreModule)
        {
            OgreRenderer::Renderer* renderer = ogreModule->GetRenderer().get();
            if (renderer)
                return renderer;
            else
                LogError("PythonScriptModule::GetRenderer(): Could not acquire valid Renderer ptr.");
        }
        else
            LogError("PythonScriptModule::GetRenderer(): Could not acquire valid OgreRenderingModule ptr.");

        return 0;
    }

    Entity* PythonScriptModule::GetActiveCamera() const
    {
        Scene *scene = GetFramework()->Scene()->GetDefaultScene().get();
        if (scene)
        {
            EntityList cameraEnts = scene->GetEntitiesWithComponent(EC_Camera::TypeNameStatic());
            EntityList::iterator iter = cameraEnts.begin();
            while (iter != cameraEnts.end())
            {
                Entity *ent = (*iter).get();
                if (ent->GetComponent<EC_Camera>()->IsActive())
                    return ent;
                ++iter;
            }

            LogWarning("PythonScriptModule::GetActiveCamera(): No active cam found from curren scene.");
        }
        else
            LogError("PythonScriptModule::GetActiveCamera(): Failed to find active camera, Scene()->GetDefaultScene() was null.");

        return 0;
    }

    Scene* PythonScriptModule::GetScene(const QString &name) const
    {
        ScenePtr scene = framework_->Scene()->GetScene(name);
        if (scene.get())
            return scene.get();
        return 0;
    }

    void PythonScriptModule::ResetQtDynamicProperty(QObject* qobj, char* propname)
    {
        if (!qobj->property(propname).isNull())
            qobj->setProperty(propname, QVariant());
    }

    InputContext* PythonScriptModule::CreateInputContext(const QString &name, int priority)
    {
        InputContextPtr newInputContext = framework_->Input()->RegisterInputContext(name, priority);
        if (newInputContext.get())
        {
            // Need to store these otherwise we get scoped ptr crash after return
            createdInputs_ << newInputContext; 
            return newInputContext.get();
        }

        return 0;
    }

    void PythonScriptModule::ShowConsole()
    {
        PythonQtScriptingConsole *console = new PythonQtScriptingConsole(0, PythonQt::self()->getMainModule(), Qt::Tool);
        if (console)
        {
            console->setAttribute(Qt::WA_DeleteOnClose, true);
            console->show();
        }
        else
            LogError("PythonScriptModule::ShowConsole(): Failed to create a PythonQtScriptingConsole with PythonQt::self()->getMainModule() context!");
    }

    void PythonScriptModule::ConsoleRunString(const QStringList &params)
    {
        const QString param = params.first();
        if (param.isEmpty())
        {
            LogError("PythonScriptModule::ConsoleRunString: Usage: PyExec(print 1 + 1)");
            return;
        }
        engine_->RunString(param);
    }

    void PythonScriptModule::ConsoleRunFile(const QStringList &params)
    {
        const QString param = params.first();
        if (param.isEmpty())
        {
            LogError("PythonScriptModule::ConsoleRunString: Usage: PyLoad(mypymodule) (to run mypymodule.py by importing it)");
            return;
        }
        engine_->RunScript(param);
    }

    void PythonScriptModule::ConsoleReset(const QStringList &params)
    {
        Uninitialize();
        Initialize();
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new PythonScript::PythonScriptModule();
        fw->RegisterModule(module);
    }
}
