// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "SceneImporter.h"
#include "SyncManager.h"

#include "SceneAPI.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"

#include "ConfigAPI.h"

#include "SceneManager.h"
#include "ConsoleCommandUtils.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "CoreStringUtils.h"
#include "AssetAPI.h"
#include "ConsoleAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"

#include "MemoryLeakCheck.h"

#include "EC_Name.h"
#include "EC_DynamicComponent.h"

#ifdef EC_Highlight_ENABLED
#include "EC_Highlight.h"
#endif

#ifdef EC_HoveringText_ENABLED
#include "EC_HoveringText.h"
#endif

#ifdef EC_Clone_ENABLED
#include "EC_Clone.h"
#endif

#ifdef EC_Touchable_ENABLED
#include "EC_Touchable.h"
#endif

#ifdef EC_3DCanvas_ENABLED
#include "EC_3DCanvas.h"
#endif

#ifdef EC_3DCanvasSource_ENABLED
#include "EC_3DCanvasSource.h"
#endif

#ifdef EC_Ruler_ENABLED
#include "EC_Ruler.h"
#endif

#ifdef EC_Sound_ENABLED
#include "EC_Sound.h"
#include "EC_SoundListener.h"
#endif

#include "EC_InputMapper.h"

#ifdef EC_VideoSource_ENABLED
#include "EC_VideoSource.h"
#endif

#ifdef EC_Gizmo_ENABLED
#include "EC_Gizmo.h"
#endif

#ifdef EC_PlanarMirror_ENABLED
#include "EC_PlanarMirror.h"
#endif

#ifdef EC_Selected_ENABLED
#include "EC_Selected.h"
#endif

#ifdef EC_ProximityTrigger_ENABLED
#include "EC_ProximityTrigger.h"
#endif

#ifdef EC_Billboard_ENABLED
#include "EC_Billboard.h"
#endif

#ifdef EC_ParticleSystem_ENABLED
#include "EC_ParticleSystem.h"
#endif

#ifdef EC_QmlApp_ENABLED
#include "EC_QmlApp.h"
#include "QmlAsset.h"
#endif

#include "EC_Camera.h"
#include "EC_Placeable.h"
#include "EC_AnimationController.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"

namespace TundraLogic
{

    static const unsigned short cDefaultPort = 2345;

TundraLogicModule::TundraLogicModule()
:IModule("TundraLogic"),
autostartserver_(false),
autostartserver_port_(cDefaultPort)
{
}

TundraLogicModule::~TundraLogicModule()
{
}

void TundraLogicModule::PreInitialize()
{
}

void TundraLogicModule::Load()
{    
    DECLARE_MODULE_EC(EC_DynamicComponent);

    // External EC's
#ifdef EC_Highlight_ENABLED
    DECLARE_MODULE_EC(EC_Highlight);
#endif
#ifdef EC_HoveringText_ENABLED
    DECLARE_MODULE_EC(EC_HoveringText);
#endif
#ifdef EC_Clone_ENABLED
    DECLARE_MODULE_EC(EC_Clone);
#endif
#ifdef EC_Touchable_ENABLED
    DECLARE_MODULE_EC(EC_Touchable);
#endif
#ifdef EC_3DCanvas_ENABLED
    DECLARE_MODULE_EC(EC_3DCanvas);
#endif
#ifdef EC_3DCanvasSource_ENABLED
    DECLARE_MODULE_EC(EC_3DCanvasSource);
#endif
#ifdef EC_Ruler_ENABLED
    DECLARE_MODULE_EC(EC_Ruler);
#endif
#ifdef EC_SoundRuler_ENABLED
    DECLARE_MODULE_EC(EC_SoundRuler);
#endif
//#ifdef EC_Name_ENABLED
    DECLARE_MODULE_EC(EC_Name);
//#endif
#ifdef EC_ParticleSystem_ENABLED
    DECLARE_MODULE_EC(EC_ParticleSystem);
#endif
#ifdef EC_Sound_ENABLED
    DECLARE_MODULE_EC(EC_Sound);
    DECLARE_MODULE_EC(EC_SoundListener);
#endif
    DECLARE_MODULE_EC(EC_InputMapper);
#ifdef EC_Movable_ENABLED
    DECLARE_MODULE_EC(EC_Movable);
#endif
#ifdef EC_VideoSource_ENABLED
    DECLARE_MODULE_EC(EC_VideoSource);
#endif
#ifdef EC_Gizmo_ENABLED
    DECLARE_MODULE_EC(EC_Gizmo);
#endif

#ifdef EC_PlanarMirror_ENABLED
    DECLARE_MODULE_EC(EC_PlanarMirror);
#endif
#ifdef EC_Selected_ENABLED
    DECLARE_MODULE_EC(EC_Selected);
#endif
#ifdef EC_Billboard_ENABLED
    DECLARE_MODULE_EC(EC_Billboard);
#endif
#ifdef EC_ProximityTrigger_ENABLED
    DECLARE_MODULE_EC(EC_ProximityTrigger);
#endif
#ifdef EC_QmlApp_ENABLED
    DECLARE_MODULE_EC(EC_QmlApp);
    framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<QmlAsset>("QML")));
#endif
}

void TundraLogicModule::Initialize()
{
    syncManager_ = boost::shared_ptr<SyncManager>(new SyncManager(this));
    client_ = boost::shared_ptr<Client>(new Client(this));
    server_ = boost::shared_ptr<Server>(new Server(this));
    
    framework_->RegisterDynamicObject("client", client_.get());
    framework_->RegisterDynamicObject("server", server_.get());
}

void TundraLogicModule::PostInitialize()
{
    framework_->Console()->RegisterCommand(CreateConsoleCommand("startserver", 
        "Starts a server. Usage: startserver(port)",
        ConsoleBind(this, &TundraLogicModule::ConsoleStartServer)));
    framework_->Console()->RegisterCommand(CreateConsoleCommand("stopserver", 
        "Stops the server",
        ConsoleBind(this, &TundraLogicModule::ConsoleStopServer)));
    framework_->Console()->RegisterCommand(CreateConsoleCommand("connect", 
        "Connects to a server. Usage: connect(address,port,username,password)",
        ConsoleBind(this, &TundraLogicModule::ConsoleConnect)));
    framework_->Console()->RegisterCommand(CreateConsoleCommand("disconnect", 
        "Disconnects from a server.",
        ConsoleBind(this, &TundraLogicModule::ConsoleDisconnect)));
    
    framework_->Console()->RegisterCommand(CreateConsoleCommand("savescene",
        "Saves scene into XML or binary. Usage: savescene(filename,binary)",
        ConsoleBind(this, &TundraLogicModule::ConsoleSaveScene)));
    framework_->Console()->RegisterCommand(CreateConsoleCommand("loadscene",
        "Loads scene from XML or binary. Usage: loadscene(filename,binary)",
        ConsoleBind(this, &TundraLogicModule::ConsoleLoadScene)));
    
    framework_->Console()->RegisterCommand(CreateConsoleCommand("importscene",
        "Loads scene from a dotscene file. Optionally clears the existing scene."
        "Replace-mode can be optionally disabled. Usage: importscene(filename,clearscene=false,replace=true)",
        ConsoleBind(this, &TundraLogicModule::ConsoleImportScene)));
    
    framework_->Console()->RegisterCommand(CreateConsoleCommand("importmesh",
        "Imports a single mesh as a new entity. Position can be specified optionally."
        "Usage: importmesh(filename,x,y,z,xrot,yrot,zrot,xscale,yscale,zscale)",
        ConsoleBind(this, &TundraLogicModule::ConsoleImportMesh)));
        
    // Take a pointer to KristalliProtocolModule so that we don't have to take/check it every time
    kristalliModule_ = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>();
    if (!kristalliModule_)
    {
        throw Exception("Fatal: could not get KristalliProtocolModule");
    }

    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_SERVER, "port", cDefaultPort, cDefaultPort);
    // Write default values to config if not present.
    if (!framework_->Config()->HasValue(configData))
        framework_->Config()->Set(configData);
    
    // Check whether server should be auto started.
    const boost::program_options::variables_map &programOptions = framework_->ProgramOptions();
    if (programOptions.count("server"))
    {
        autostartserver_ = true;
        // Use parameter port or default to config value
        if (programOptions.count("port"))
        {
            try
            {
                autostartserver_port_ = programOptions["port"].as<int>();
            }
            catch(...)
            {
                LogFatal("--port parameter is not a valid integer.");
                GetFramework()->Exit();
            }
        }
        else
            autostartserver_port_ = GetFramework()->Config()->Get(configData).toInt();
    }
}

void TundraLogicModule::Uninitialize()
{
    kristalliModule_.reset();
    syncManager_.reset();
    client_.reset();
    server_.reset();
}

void TundraLogicModule::Update(f64 frametime)
{
    {
        PROFILE(TundraLogicModule_Update);
        
        static bool check_default_server_start = true;
        if (check_default_server_start)
        {
            if (autostartserver_)
                server_->Start(autostartserver_port_);

            // Load startup scene here (if we have one)
            LoadStartupScene();
            
            check_default_server_start = false;
        }

        static bool check_login_start = true;
        if (check_login_start)
        {
            // Web login handling, if we are on a server the request will be ignored down the chain.
            const boost::program_options::variables_map &options = GetFramework()->ProgramOptions();
            if (options.count("login") > 0)
            {
                LogInfo(QString::fromStdString(options["login"].as<std::string>()).toStdString());
                QUrl loginUrl(QString::fromStdString(options["login"].as<std::string>()), QUrl::TolerantMode);
                if (loginUrl.isValid())
                    client_->Login(loginUrl);
            }

            check_login_start = false;
        }
        
        // Update client & server
        if (client_)
            client_->Update(frametime);
        if (server_)
            server_->Update(frametime);
        // Run scene sync
        if (syncManager_)
            syncManager_->Update(frametime);
        // Run scene interpolation
        ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
        if (scene)
            scene->UpdateAttributeInterpolations(frametime);
    }
    
    RESETPROFILER;
}

void TundraLogicModule::LoadStartupScene()
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        scene = framework_->Scene()->CreateScene("TundraServer", true);
        framework_->Scene()->SetDefaultScene(scene);
    }
    
    const boost::program_options::variables_map &options = GetFramework()->ProgramOptions();
    if (options.count("file") == 0)
        return; // No startup scene specified, ignore.

    std::string startupScene = QString(options["file"].as<std::string>().c_str()).trimmed().toStdString();
    if (startupScene.empty())
        return; // No startup scene specified, ignore.

    // At this point, if we have a LocalAssetProvider, it has already also parsed the --file command line option
    // and added the appropriate path as a local asset storage. Here we assume that is the case, so that the
    // scene we now load will be able to refer to local:// assets in its subfolders.
    AssetAPI::AssetRefType sceneRefType = AssetAPI::ParseAssetRef(QString::fromStdString(startupScene));
    if (sceneRefType != AssetAPI::AssetRefLocalPath && sceneRefType != AssetAPI::AssetRefRelativePath)
    {
        AssetTransferPtr sceneTransfer = framework_->Asset()->RequestAsset(startupScene.c_str());
        if (!sceneTransfer.get())
        {
            LogError("Asset transfer initialization failed for scene file " + startupScene + " failed");
            return;
        }
        connect(sceneTransfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(StartupSceneLoaded(AssetPtr)));
        connect(sceneTransfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), SLOT(StartupSceneTransferFailed(IAssetTransfer*, QString)));
        LogInfo("[TundraLogic] Loading startup scene from " + startupScene);
    }
    else
    {
        LogInfo("[TundraLogic] Loading startup scene from " + startupScene);
        bool useBinary = startupScene.find(".tbin") != std::string::npos;
        if (!useBinary)
            scene->LoadSceneXML(startupScene.c_str(), true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
        else
            scene->LoadSceneBinary(startupScene.c_str(), true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
    }
}

void TundraLogicModule::StartupSceneLoaded(AssetPtr asset)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return;

    QString sceneDiskSource = asset->DiskSource();
    if (!sceneDiskSource.isEmpty())
    {
        bool useBinary = sceneDiskSource.endsWith(".tbin");
        if (!useBinary)
            scene->LoadSceneXML(sceneDiskSource, true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
        else
            scene->LoadSceneBinary(sceneDiskSource, true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
    }
    else
        LogError("Could not resolve disk source for loaded scene file " + asset->Name().toStdString());
}

void TundraLogicModule::StartupSceneTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("Failed to load startup scene from " + transfer->GetSourceUrl().toStdString() + " reason: " + reason.toStdString());
}

ConsoleCommandResult TundraLogicModule::ConsoleStartServer(const StringVector& params)
{
    unsigned short port = cDefaultPort;
    
    try
    {
        if (params.size() > 0)
            port = ParseString<int>(params[0]);
    }
    catch(...) {}
    
    server_->Start(port);
    
    return ConsoleResultSuccess();
}

ConsoleCommandResult TundraLogicModule::ConsoleStopServer(const StringVector& params)
{
    server_->Stop();
    
    return ConsoleResultSuccess();
}

ConsoleCommandResult TundraLogicModule::ConsoleConnect(const StringVector& params)
{
    if (params.size() < 1)
        return ConsoleResultFailure("No address specified");
    
    unsigned short port = cDefaultPort;
    std::string username = "test";
    std::string password = "test";
    
    try
    {
        if (params.size() > 1)
            port = ParseString<int>(params[1]);
        if (params.size() > 2)
            username = params[2];
        if (params.size() > 3)
            password = params[3];
    }
    catch(...) {}
    
    client_->Login(QString::fromStdString(params[0]), port, QString::fromStdString(username), QString::fromStdString(password));
    
    return ConsoleResultSuccess();
}

ConsoleCommandResult TundraLogicModule::ConsoleDisconnect(const StringVector& params)
{
    client_->Logout(false);
    
    return ConsoleResultSuccess();
}

ConsoleCommandResult TundraLogicModule::ConsoleSaveScene(const StringVector &params)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return ConsoleResultFailure("No active scene found.");
    if (params.size() < 1)
        return ConsoleResultFailure("No filename given.");
    
    bool useBinary = false;
    if ((params.size() > 1) && (params[1] == "binary"))
        useBinary = true;
    
    bool success;
    if (!useBinary)
        success = scene->SaveSceneXML(params[0].c_str());
    else
        success = scene->SaveSceneBinary(params[0].c_str());
    
    if (success)
        return ConsoleResultSuccess();
    else
        return ConsoleResultFailure("Failed to save the scene.");
}

ConsoleCommandResult TundraLogicModule::ConsoleLoadScene(const StringVector &params)
{
    ///\todo Add loadScene parameter
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return ConsoleResultFailure("No active scene found.");
    if (params.size() < 1)
        return ConsoleResultFailure("No filename given.");
    
    bool useBinary = false;
    if ((params.size() > 1) && (params[1] == "binary"))
        useBinary = true;
    
    QList<Entity *> entities;
    if (!useBinary)
        entities = scene->LoadSceneXML(params[0].c_str(), true/*clearScene*/, false/*replaceOnConflcit*/, AttributeChange::Default);
    else
        entities = scene->LoadSceneBinary(params[0].c_str(), true/*clearScene*/, false/*replaceOnConflcit*/, AttributeChange::Default);
    
    if (!entities.empty())
    {
        return ConsoleResultSuccess();
    }
    else
        return ConsoleResultFailure("Failed to load the scene.");
}

ConsoleCommandResult TundraLogicModule::ConsoleImportScene(const StringVector &params)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return ConsoleResultFailure("No active scene found.");
    if (params.size() < 1)
        return ConsoleResultFailure("No filename given.");
    bool clearscene = false;
    bool replace = true;
    if (params.size() > 1)
        clearscene = ParseBool(params[1]);
    if (params.size() > 2)
        replace = ParseBool(params[2]);
    
    std::string filename = params[0];
    boost::filesystem::path path(filename);
    std::string dirname = path.branch_path().string();
    
    SceneImporter importer(scene);
    QList<Entity *> entities = importer.Import(filename, dirname, Transform(),
        "local://", AttributeChange::Default, clearscene, replace);
    if (!entities.empty())
    {
        return ConsoleResultSuccess();
    }
    else
        return ConsoleResultFailure("Failed to import the scene.");
}

ConsoleCommandResult TundraLogicModule::ConsoleImportMesh(const StringVector &params)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return ConsoleResultFailure("No active scene found.");
    if (params.size() < 1)
        return ConsoleResultFailure("No filename given.");
    
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float xr = 0.0f, yr = 0.0f, zr = 0.0f;
    float xs = 1.0f, ys = 1.0f,zs = 1.0f;
    if (params.size() >= 4)
    {
        x = ParseString<float>(params[1], 0.0f);
        y = ParseString<float>(params[2], 0.0f);
        z = ParseString<float>(params[3], 0.0f);
    }
    if (params.size() >= 7)
    {
        xr = ParseString<float>(params[4], 0.0f);
        yr = ParseString<float>(params[5], 0.0f);
        zr = ParseString<float>(params[6], 0.0f);
    }
    if (params.size() >= 10)
    {
        xs = ParseString<float>(params[7], 1.0f);
        ys = ParseString<float>(params[8], 1.0f);
        zs = ParseString<float>(params[9], 1.0f);
    }
    
    std::string filename = params[0];
    boost::filesystem::path path(filename);
    std::string dirname = path.branch_path().string();
    
    SceneImporter importer(scene);
    EntityPtr entity = importer.ImportMesh(filename, dirname, Transform(Vector3df(x,y,z),
        Vector3df(xr,yr,zr), Vector3df(xs,ys,zs)), std::string(), "local://", AttributeChange::Default, true);
    
    return ConsoleResultSuccess();
}

bool TundraLogicModule::IsServer() const
{
    return kristalliModule_->IsServer();
}

}

void SetProfiler(Profiler *profiler)
{
    ProfilerSection::SetProfiler(profiler);
}

using namespace TundraLogic;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    IModule *module = new TundraLogic::TundraLogicModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
