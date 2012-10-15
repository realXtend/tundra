// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

// PythonScriptModule
#include "PythonScriptModule.h"
#include "LoggingFunctions.h"

// Framework and APIs
#include "Framework.h"
#include "Application.h"
#include "CoreTypes.h"
#include "CoreDefines.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "InputAPI.h"
#include "FrameAPI.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "AudioAPI.h"
#include "PluginAPI.h"
#include "UiAPI.h"

/// Framework, SceneAPI and Kristalli
#include "Application.h"
#include "IRenderer.h"
#include "Scene.h"
#include "Entity.h"
#include "IComponent.h"
#include "IComponentFactory.h"
#include "IAttribute.h"

// AssetAPI
#include "AssetCache.h"
#include "AssetReference.h"
#include "IAssetTypeFactory.h"
#include "IAsset.h"
#include "IAssetProvider.h"
#include "IAssetStorage.h"
#include "IAssetTransfer.h"
#include "IAssetUploadTransfer.h"
#include "GenericAssetFactory.h"

// InputAPI
#include "InputContext.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "GestureEvent.h"

// UiAPI
#include "UiGraphicsView.h"
#include "UiMainWindow.h"
#include "UiProxyWidget.h"

// TundraLogicModule
#include "TundraLogicModule.h"
#include "UserConnection.h"
#include "Client.h"
#include "Server.h"

// Python and PythonQt
#include <PythonQt.h>
#include <PythonQt_QtAll.h>
#include <PythonQtSignalReceiver.h>
#include "PythonScriptInstance.h"
#include "PythonQtScriptingConsole.h"
#include "TundraWrapper.h"

// OgreRenderingModule
#include "OgreRenderingModule.h"
#include "Renderer.h"

// ECs from SDK + OgreRenderingModule
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_Camera.h"
#include "EC_Material.h"
#include "EC_ParticleSystem.h"
#include "EC_AnimationController.h"
#include "EC_DynamicComponent.h"
#include "EC_Name.h"
#include "EC_OgreCompositor.h"
#include "EC_InputMapper.h"

// EC_Script
#include "ScriptAsset.h"
#include "EC_Script.h"
#include "ScriptAsset.h"

// Conditional ECs
#include "EntityComponentDefines.h"

// Qt
#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QStringList>

#include "MemoryLeakCheck.h" // Keep this as the last include

namespace PythonScript
{
    PythonScriptModule::PythonScriptModule() :
        IModule("PythonScript"),
        pythonQtStarted_(false)
    {
        StartPythonQt();
    }

    PythonScriptModule::~PythonScriptModule()
    {
    }

    void PythonScriptModule::Uninitialize()
    {
        // Clear script created input contexts.
        createdInputs_.clear();

        PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
        if (!mainModule.isNull())
        {
            mainModule.removeVariable("_pythonscriptmodule");
            mainModule.removeVariable("_tundra");
        }

        // This will remove all the signal handlers in PythonQt.
        // This function is only available in the modified PythonQt realXtend Tundra made.
        // Otherwise our app will crash when deleting the framework APIs as there are connected signals still to python slots.
        LogInfo("PythonScriptModule: Disconnecting all PythonQt connected signals");
        PythonQt::priv()->disconnectAllSignalReceivers();
        PythonQt::priv()->deleteAllSignalReceivers();

        // Note that we do not call Py_Finalize() before or after PythonQt::cleanup()
        // as this will crash either way after doing the above. Let python release its memory when the dll is unloaded.
        LogInfo("PythonScriptModule: Running PythonQt cleanup");
        PythonQt::cleanup();
    }

    void PythonScriptModule::Load()
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

    void PythonScriptModule::Initialize()
    {
        // An error has occurred on startup.
        if (!pythonQtStarted_)
            return;

        // Get python main module.
        PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
        if (mainModule.isNull())
        {
            LogError("PythonScriptModule::StartPythonQt(): Failed to get main module from PythonQt after init!");
            return;
        }

        // Add PythonScriptModule as '_pythonscriptmodule' 
        // and Framework as '_tundra' to python.
        mainModule.addObject("_pythonscriptmodule", this);
        mainModule.addObject("_tundra", GetFramework());

        QDir pythonPlugins(Application::InstallationDirectory() + "pyplugins");
        QDir pythonLibrary(pythonPlugins.absoluteFilePath("python/"));

        // Add Tundra python plugins source location.
        AddSystemPath(pythonPlugins.absolutePath());
        AddSystemPath(pythonPlugins.absoluteFilePath("lib"));

        // Add Python Library DLL and on windows pass whole python as a archive file.
#ifdef _WIN32            
        AddSystemPath(pythonLibrary.absoluteFilePath("DLLs"));
        AddSystemPath(pythonLibrary.absoluteFilePath("Python26.zip"));
#endif

        // Connect to SceneAPI
        QObject::connect(GetFramework()->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString&)));

        // Console commands to ConsoleAPI
        GetFramework()->Console()->RegisterCommand("PyExec", "Execute given code in the embedded Python interpreter. Usage: PyExec(mycodestring)", 
                                                   this, SLOT(ConsoleRunString(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyLoad", "Execute a python file. Usage: PyLoad(mypymodule)", 
                                                   this, SLOT(ConsoleRunFile(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyRestart", "Restarts the Tundra Python ModuleManager", 
                                                   this, SLOT(ConsoleRestartPython(const QStringList&)));
        GetFramework()->Console()->RegisterCommand("PyConsole", "Creates a new Python console window.", 
                                                   this, SLOT(ShowConsole()));

        // Done in PostInitialize() so all modules/APIs are loaded and initialized.
        //StartPythonModuleManager();
/*        
        // --p --python --pythonapitests are special command line options that on purpose not put
        // to the Framework program options parsing. So lets do a special parse here for there hidden variables.
        /// \todo See if we should return --p --python as official cmd line options back to Framework. Probably best to have modules give own params somehow, 
        /// we should not mess python specific things to core SDK params in Framework.cpp :I
        namespace po = boost::program_options;

        po::variables_map commandLineVariables;
        po::options_description commandLineDescriptions;

        commandLineDescriptions.add_options()
            ("p", po::value<std::string>(), "Run a python script on startup")
            ("python", po::value<std::string>(), "Run a python script on startup")
            ("pythonapitests", "Run a python api test script on startup");

        try
        {
            /// \note QApplication::argc() and QApplication::argv() are deprecated.
            po::store(po::command_line_parser(QApplication::argc(), QApplication::argv()).options(commandLineDescriptions).allow_unregistered().run(), commandLineVariables);
        }
        catch(std::exception &e)
        {
            LogWarning(Name() + ": " + + e.what());
        }
        po::notify(commandLineVariables);
*/
        if (GetFramework()->HasCommandLineParameter("--python") || GetFramework()->HasCommandLineParameter("--p"))
        {
            QStringList scripts = GetFramework()->CommandLineParameters("--python");
            scripts.append(GetFramework()->CommandLineParameters("--p"));
            foreach(QString script, scripts)
                RunScript(script);
        }

        LoadStartupScripts();
    }

    void PythonScriptModule::LoadStartupScripts()
    {
        QStringList pluginsToLoad;
        foreach(const QString &configFile, GetFramework()->Plugins()->ConfigurationFiles())
        {
            QDomDocument doc("plugins");
            QFile file(configFile);
            if (!file.open(QIODevice::ReadOnly))
            {
                LogError("PythonScriptModule::LoadStartupScripts: Failed to open file \"" + configFile + "\"!");
                return;
            }
            if (!doc.setContent(&file))
            {
                LogError("PythonScriptModule::LoadStartupScripts: Failed to parse XML file \"" + configFile + "\"!");
                file.close();
                return;
            }
            file.close();

            QDomElement docElem = doc.documentElement();

            QDomNode n = docElem.firstChild();
            while(!n.isNull())
            {
                QDomElement e = n.toElement(); // try to convert the node to an element.
                if (!e.isNull() && e.tagName() == "pyplugin" && e.hasAttribute("path"))
                    pluginsToLoad.push_back(e.attribute("path"));
                n = n.nextSibling();
            }
        }

        LogInfo(Name() + ": Loading scripts from startup config");
        if (pluginsToLoad.empty())
            LogInfo(Name() + ": ** No scripts in config");

        QDir pythonPlugins(Application::InstallationDirectory() + "pyplugins");
        foreach(QString pluginPath, pluginsToLoad)
        {
            const QString pluginFile = pythonPlugins.absoluteFilePath(pluginPath);
            if (QFile::exists(pluginFile))
            {
                LogInfo(Name() + ": ** " + pluginPath);
                RunScript(pluginFile);
            }
            else
                LogWarning(Name() + ": ** Could not locate startup pyplugin '" + pluginPath +"'. Make sure your path is relative to /pyplugins folder.");
        }

    }

    void PythonScriptModule::StartPythonQt()
    {
        if (!pythonQtStarted_)
        {
            Py_NoSiteFlag = 1;
            Py_InitializeEx(0);

            PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut | PythonQt::PythonAlreadyInitialized);
            PythonQt_QtAll::init();
            if (!Py_IsInitialized())
            {
                LogError("PythonScriptModule::StartPythonQt(): Could not Py_Initialize python!");
                return;
            }

            QObject::connect(PythonQt::self(), SIGNAL(pythonStdOut(const QString&)), this, SLOT(OnPythonQtStdOut(const QString&)));
            QObject::connect(PythonQt::self(), SIGNAL(pythonStdErr(const QString&)), this, SLOT(OnPythonQtStdErr(const QString&)));
            
            /// \todo We need a way for other modules to register ECs/Classes 
            /// to PythonModule so it does not have to link against everything in /src/Application.

            // Framework and APIs
            PythonQt::self()->registerClass(&Framework::staticMetaObject);
            PythonQt::self()->registerClass(&Application::staticMetaObject);
            PythonQt::self()->registerClass(&SceneAPI::staticMetaObject);
            PythonQt::self()->registerClass(&AssetAPI::staticMetaObject);
            PythonQt::self()->registerClass(&InputAPI::staticMetaObject);
            PythonQt::self()->registerClass(&FrameAPI::staticMetaObject);
            PythonQt::self()->registerClass(&ConsoleAPI::staticMetaObject);
            PythonQt::self()->registerClass(&ConfigAPI::staticMetaObject);
            PythonQt::self()->registerClass(&AudioAPI::staticMetaObject);
            PythonQt::self()->registerClass(&UiAPI::staticMetaObject);

            // ECs
            PythonQt::self()->registerClass(&EC_Camera::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Mesh::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Placeable::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Material::staticMetaObject);
            PythonQt::self()->registerClass(&EC_ParticleSystem::staticMetaObject);
            PythonQt::self()->registerClass(&EC_AnimationController::staticMetaObject);
            PythonQt::self()->registerClass(&EC_DynamicComponent::staticMetaObject);
            PythonQt::self()->registerClass(&EC_Name::staticMetaObject);
            PythonQt::self()->registerClass(&EC_OgreCompositor::staticMetaObject);
            PythonQt::self()->registerClass(&EC_InputMapper::staticMetaObject);

            // Conditional ECs
#ifdef EC_Highlight_ENABLED
            PythonQt::self()->registerClass(&EC_Highlight::staticMetaObject);
#endif
#ifdef EC_HoveringText_ENABLED
            PythonQt::self()->registerClass(&EC_HoveringText::staticMetaObject);
#endif
#ifdef EC_Sound_ENABLED
            PythonQt::self()->registerClass(&EC_Sound::staticMetaObject);
            PythonQt::self()->registerClass(&EC_SoundListener::staticMetaObject);
#endif
#ifdef EC_PlanarMirror_ENABLED
            PythonQt::self()->registerClass(&EC_PlanarMirror::staticMetaObject);
#endif
#ifdef EC_ProximityTrigger_ENABLED
            PythonQt::self()->registerClass(&EC_ProximityTrigger::staticMetaObject);
#endif
#ifdef EC_Billboard_ENABLED
            PythonQt::self()->registerClass(&EC_Billboard::staticMetaObject);
#endif
#ifdef EC_ParticleSystem_ENABLED
            PythonQt::self()->registerClass(&EC_ParticleSystem::staticMetaObject);
#endif
#ifdef EC_TransformGizmo_ENABLED
            PythonQt::self()->registerClass(&EC_TransformGizmo::staticMetaObject);
#endif

            // Scene
            PythonQt::self()->registerClass(&Scene::staticMetaObject);
            PythonQt::self()->registerClass(&Entity::staticMetaObject);
            PythonQt::self()->registerClass(&IComponent::staticMetaObject);
            PythonQt::self()->registerClass(&EntityAction::staticMetaObject);
            PythonQt::self()->registerClass(&AttributeChange::staticMetaObject);

            // Asset
            PythonQt::self()->registerClass(&AssetCache::staticMetaObject);
            PythonQt::self()->registerClass(&IAsset::staticMetaObject);
            PythonQt::self()->registerClass(&IAssetTransfer::staticMetaObject);
            PythonQt::self()->registerClass(&IAssetStorage::staticMetaObject);
            PythonQt::self()->registerClass(&IAssetUploadTransfer::staticMetaObject);

            // Rendering
            PythonQt::self()->registerClass(&OgreRenderer::OgreRenderingModule::staticMetaObject);
            PythonQt::self()->registerClass(&OgreRenderer::Renderer::staticMetaObject);
            PythonQt::self()->registerClass(&RaycastResult::staticMetaObject);

            // Ui
            PythonQt::self()->registerClass(&UiMainWindow::staticMetaObject);
            PythonQt::self()->registerClass(&UiGraphicsView::staticMetaObject);
            PythonQt::self()->registerClass(&UiProxyWidget::staticMetaObject);

            // Input
            PythonQt::self()->registerClass(&InputContext::staticMetaObject);
            PythonQt::self()->registerClass(&KeyEvent::staticMetaObject);
            PythonQt::self()->registerClass(&MouseEvent::staticMetaObject);
            PythonQt::self()->registerClass(&GestureEvent::staticMetaObject);

            // TundraLogicModule
            PythonQt::self()->registerClass(&UserConnection::staticMetaObject);
            PythonQt::self()->registerClass(&TundraLogic::TundraLogicModule::staticMetaObject);
            PythonQt::self()->registerClass(&TundraLogic::Client::staticMetaObject);
            PythonQt::self()->registerClass(&TundraLogic::Server::staticMetaObject);

            // Misc
            PythonQt::self()->registerClass(&ConsoleCommand::staticMetaObject);
            PythonQt::self()->registerClass(&DelayedSignal::staticMetaObject);
            PythonQt::self()->registerClass(&PythonQtScriptingConsole::staticMetaObject); // ??

            // Decorators
            /// \todo Reimplemented decorators for new math lib
            PythonQt::self()->addInstanceDecorators(new TundraInstanceDecorator());
            PythonQt::self()->addDecorators(new TundraDecorator());
            PythonQt::self()->registerCPPClass("AssetReference");
            PythonQt::self()->registerCPPClass("ScenePtr");

            pythonQtStarted_ = true;
        }
    }

    void PythonScriptModule::AddSystemPath(const QString &path)
    {
        RunString("import sys; sys.path.append('" + path + "');");
    }

    void PythonScriptModule::RunString(const QString &codeStr)
    {
        if (!Py_IsInitialized())
            return;
        
        PyRun_SimpleString(codeStr.toAscii().data());
    }

    void PythonScriptModule::RunScript(const QString &scriptname)
    {
        if (!Py_IsInitialized())
            return;

        FILE *fp = fopen(scriptname.toAscii().data(), "r");
        if (!fp)
        {
            LogInfo("PythonEngine::RunScript(): Failed to open script " + scriptname.toStdString());
            return;
        }

        PyRun_SimpleFile(fp, scriptname.toAscii().data());
        fclose(fp);
    }

    void PythonScriptModule::OnPythonQtStdOut(const QString &str)
    {
        QString strSimplified = str.trimmed();
        if (!strSimplified.isEmpty())
            LogInfo(Name() + ": " + strSimplified);
    }

    void PythonScriptModule::OnPythonQtStdErr(const QString &str)
    {
        QString strSimplified = str.trimmed();
        if (!strSimplified.isEmpty())
            LogError(Name() + ": " + strSimplified);
    }

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
            QObject::connect(component, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), 
                             this, SLOT(LoadScripts(const std::vector<ScriptAssetPtr>&)), Qt::UniqueConnection);
    }

    void PythonScriptModule::OnComponentRemoved(Entity *entity, IComponent *component)
    {
        if (component->TypeName() == EC_Script::TypeNameStatic())
            QObject::disconnect(component, SIGNAL(ScriptAssetsChanged(const std::vector<ScriptAssetPtr>&)), this, SLOT(LoadScripts(const std::vector<ScriptAssetPtr>&)));
    }

    void PythonScriptModule::LoadScripts(const std::vector<ScriptAssetPtr> &newScripts)
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

    QObject *PythonScriptModule::GetServer() const
    {
        QVariant serverVariant = GetFramework()->property("server");
        if (serverVariant.canConvert<QObject*>())
        {
            QObject *serverPtr = serverVariant.value<QObject*>();
            return serverPtr;
        }
        return 0;
    }

    QObject *PythonScriptModule::GetClient() const
    {
        QVariant clientVariant = GetFramework()->property("client");
        if (clientVariant.canConvert<QObject*>())
        {
            QObject *clientPtr = clientVariant.value<QObject*>();
            return clientPtr;
        }
        return 0;
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
        Scene *scene = GetFramework()->Scene()->MainCameraScene();
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
        RunString(param);
    }

    void PythonScriptModule::ConsoleRunFile(const QStringList &params)
    {
        const QString param = params.first();
        if (param.isEmpty())
        {
            LogError("PythonScriptModule::ConsoleRunString: Usage: PyLoad(mypymodule) (to run mypymodule.py by importing it)");
            return;
        }
        RunScript(param);
    }

    void PythonScriptModule::ConsoleRestartPython(const QStringList &params)
    {
        /// \todo Reset current python plugins and call LoadStartupScripts again.
        LogWarning("PythonScriptModule::ConsoleRestartPython not implemented yet!");
    }

    void PythonScriptModule::PythonPrintLog(const QString &logType, const QString &logMessage)
    {
        const QString message = Name() + ": " + logMessage;
        const QString logTypeUpper = logType.toUpper();
        if (logTypeUpper == "WARNING")
            LogWarning(message);
        else if (logTypeUpper == "ERROR")
            LogError(message);
        else if (logTypeUpper == "DEBUG")
            LogDebug(message);
        else
            LogInfo(message);
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
