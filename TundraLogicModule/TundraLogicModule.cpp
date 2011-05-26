// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "SceneImporter.h"
#include "SyncManager.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "Profiler.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "ConfigAPI.h"
#include "IComponentFactory.h"
#include "Scene.h"

#include "KristalliProtocolModule.h"
#include "CoreStringUtils.h"
#include "AssetAPI.h"
#include "ConsoleAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "CoreException.h"
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

#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif

#include "EC_Camera.h"
#include "EC_Placeable.h"
#include "EC_AnimationController.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"

#include <boost/filesystem.hpp>

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
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_DynamicComponent>));

    // External EC's
#ifdef EC_Highlight_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Highlight>));
#endif
#ifdef EC_HoveringText_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_HoveringText>));
#endif
#ifdef EC_Clone_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Clone>));
#endif
#ifdef EC_Touchable_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Touchable>));
#endif
#ifdef EC_3DCanvas_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_3DCanvas>));
#endif
#ifdef EC_3DCanvasSource_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_3DCanvasSource>));
#endif
#ifdef EC_Ruler_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Ruler>));
#endif
#ifdef EC_SoundRuler_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_SoundRuler>));
#endif
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Name>));
#ifdef EC_ParticleSystem_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_ParticleSystem>));
#endif
#ifdef EC_Sound_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Sound>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_SoundListener>));
#endif
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_InputMapper>));
#ifdef EC_Movable_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Movable>));
#endif
#ifdef EC_VideoSource_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_VideoSource>));
#endif
#ifdef EC_Gizmo_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Gizmo>));
#endif

#ifdef EC_PlanarMirror_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_PlanarMirror>));
#endif
#ifdef EC_Selected_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Selected>));
#endif
#ifdef EC_Billboard_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Billboard>));
#endif
#ifdef EC_ProximityTrigger_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_ProximityTrigger>));
#endif
#ifdef EC_TransformGizmo_ENABLED
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_TransformGizmo>));
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
    framework_->Console()->RegisterCommand("startserver", "Starts a server. Usage: startserver(port)",
        this, SLOT(StartServer(int)));

    framework_->Console()->RegisterCommand("stopserver", "Stops the server",
        this, SLOT(StopServer()));

    framework_->Console()->RegisterCommand("connect", "Connects to a server. Usage: connect(address,port,username,password)",
        this, SLOT(Connect(QString,int,QString,QString)));

    framework_->Console()->RegisterCommand("disconnect", "Disconnects from a server.",
        this, SLOT(Disconnect()));

    framework_->Console()->RegisterCommand("savescene", "Saves scene into XML or binary. Usage: savescene(filename,binary)",
        this, SLOT(SaveScene(QString, bool, bool, bool)));

    framework_->Console()->RegisterCommand("loadscene", "Loads scene from XML or binary. Usage: loadscene(filename,binary)",
        this, SLOT(ConsoleLoadSceneLoadScene(QString, bool, bool)));

    framework_->Console()->RegisterCommand("importscene",
        "Loads scene from a dotscene file. Optionally clears the existing scene."
        "Replace-mode can be optionally disabled. Usage: importscene(filename,clearscene=false,replace=true)",
        this, SLOT(ImportScene(QString, bool, bool)));

    framework_->Console()->RegisterCommand("importmesh",
        "Imports a single mesh as a new entity. Position can be specified optionally."
        "Usage: importmesh(filename,x,y,z,xrot,yrot,zrot,xscale,yscale,zscale)",
        this, SLOT(ImportMesh(QString, float, float, float, float, float, float, float, float, float, bool)));

    // Take a pointer to KristalliProtocolModule so that we don't have to take/check it every time
    kristalliModule_ = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
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
    kristalliModule_ = 0;
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
    
}

void TundraLogicModule::LoadStartupScene()
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        scene = framework_->Scene()->CreateScene("TundraServer", true, true);

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

void TundraLogicModule::StartServer(int port)
{
    server_->Start(port);
}

void TundraLogicModule::StopServer()
{
    server_->Stop();
}

void TundraLogicModule::Connect(QString address, int port, QString username, QString password)
{
    client_->Login(address, port, username, password);
}

void TundraLogicModule::Disconnect()
{
    client_->Logout(false);
}

void TundraLogicModule::SaveScene(QString filename, bool asBinary, bool saveTemporaryEntities, bool saveLocalEntities)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogError("No active scene found!");
        return;
    }
    filename = filename.trimmed();
    if (filename.isEmpty())
    {
        LogError("Empty filename given!");
        return;
    }
    
    bool success;
    if (!asBinary)
        success = scene->SaveSceneXML(filename, saveTemporaryEntities, saveLocalEntities);
    else
        success = scene->SaveSceneBinary(filename, saveTemporaryEntities, saveLocalEntities);

    if (!success)
        LogError("SaveScene failed!");
}

void TundraLogicModule::LoadScene(QString filename, bool clearScene, bool useEntityIDsFromFile)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogError("No active scene found!");
        return;
    }
    filename = filename.trimmed();
    if (filename.isEmpty())
    {
        LogError("Empty filename given!");
        return;
    }
    
    bool useBinary = false;
    if (filename.contains(".tbin", Qt::CaseInsensitive))
        useBinary = true;

    QList<Entity *> entities;
    if (!useBinary)
        entities = scene->LoadSceneXML(filename, clearScene, useEntityIDsFromFile, AttributeChange::Default);
    else
        entities = scene->LoadSceneBinary(filename, clearScene, useEntityIDsFromFile, AttributeChange::Default);

    LogInfo("Loaded " + QString::number(entities.size()) + " entities.");
}

void TundraLogicModule::ImportScene(QString filename, bool clearScene, bool replace)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogError("No active scene found!");
        return;
    }
    filename = filename.trimmed();
    if (filename.isEmpty())
    {
        LogError("Empty filename given!");
        return;
    }
    
    ///\todo Lacks unicode support. -jj.
    boost::filesystem::path path(filename.toStdString());
    std::string dirname = path.branch_path().string();
    
    SceneImporter importer(scene);
    QList<Entity *> entities = importer.Import(filename.toStdString(), dirname, Transform(),
        "local://", AttributeChange::Default, clearScene, replace);

    LogInfo("Imported " + QString::number(entities.size()) + " entities.");
}

void TundraLogicModule::ImportMesh(QString filename, float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz, bool inspect)
{
    ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
    {
        LogError("No active scene found!");
        return;
    }
    filename = filename.trimmed();
    if (filename.isEmpty())
    {
        LogError("Empty filename given!");
        return;
    }

    ///\todo Lacks unicode support. -jj.
    boost::filesystem::path path(filename.toStdString());
    std::string dirname = path.branch_path().string();
    
    SceneImporter importer(scene);
    EntityPtr entity = importer.ImportMesh(filename.toStdString(), dirname, Transform(Vector3df(tx,ty,tz),
        Vector3df(rx,ry,rz), Vector3df(sx,sy,sz)), std::string(), "local://", AttributeChange::Default, inspect);
}

bool TundraLogicModule::IsServer() const
{
    return kristalliModule_->IsServer();
}

}

using namespace TundraLogic;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new TundraLogic::TundraLogicModule();
    fw->RegisterModule(module);
}
}
