// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "TundraEvents.h"
#include "SceneImporter.h"
#include "SyncManager.h"

#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "CoreStringUtils.h"
#include "AssetServiceInterface.h"
#include "LocalAssetProvider.h"
#include "AssetAPI.h"

#include "MemoryLeakCheck.h"

namespace TundraLogic
{

std::string TundraLogicModule::type_name_static_ = "TundraLogic";

static const unsigned short cDefaultPort = 2345;

TundraLogicModule::TundraLogicModule() : IModule(type_name_static_)
{
}

TundraLogicModule::~TundraLogicModule()
{
}

void TundraLogicModule::PreInitialize()
{
}

void TundraLogicModule::Initialize()
{
    tundraEventCategory_ = framework_->GetEventManager()->RegisterEventCategory("Tundra");
    
    syncManager_ = boost::shared_ptr<SyncManager>(new SyncManager(this, framework_));
    client_ = boost::shared_ptr<Client>(new Client(this, framework_));
    server_ = boost::shared_ptr<Server>(new Server(this, framework_));
    
    framework_->RegisterDynamicObject("client", client_.get());
    framework_->RegisterDynamicObject("server", server_.get());
}

void TundraLogicModule::PostInitialize()
{
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");
    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    
    RegisterConsoleCommand(Console::CreateCommand("startserver", 
        "Starts a server. Usage: startserver(port)",
        Console::Bind(this, &TundraLogicModule::ConsoleStartServer)));
    RegisterConsoleCommand(Console::CreateCommand("stopserver", 
        "Stops the server",
        Console::Bind(this, &TundraLogicModule::ConsoleStopServer)));
    RegisterConsoleCommand(Console::CreateCommand("connect", 
        "Connects to a server. Usage: connect(address,port,username,password)",
        Console::Bind(this, &TundraLogicModule::ConsoleConnect)));
    RegisterConsoleCommand(Console::CreateCommand("disconnect", 
        "Disconnects from a server.",
        Console::Bind(this, &TundraLogicModule::ConsoleDisconnect)));
    
    RegisterConsoleCommand(Console::CreateCommand("savescene",
        "Saves scene into XML or binary. Usage: savescene(filename,binary)",
        Console::Bind(this, &TundraLogicModule::ConsoleSaveScene)));
    RegisterConsoleCommand(Console::CreateCommand("loadscene",
        "Loads scene from XML or binary. Usage: loadscene(filename,binary)",
        Console::Bind(this, &TundraLogicModule::ConsoleLoadScene)));
    
    RegisterConsoleCommand(Console::CreateCommand("importscene",
        "Loads scene from a dotscene file. Optionally clears the existing scene."
        "Replace-mode can be optionally disabled. Usage: importscene(filename,clearscene=false,replace=true)",
        Console::Bind(this, &TundraLogicModule::ConsoleImportScene)));
    
    RegisterConsoleCommand(Console::CreateCommand("importmesh",
        "Imports a single mesh as a new entity. Position can be specified optionally."
        "Usage: importmesh(filename,x,y,z,xrot,yrot,zrot,xscale,yscale,zscale)",
        Console::Bind(this, &TundraLogicModule::ConsoleImportMesh)));
        
    // Take a pointer to KristalliProtocolModule so that we don't have to take/check it every time
    kristalliModule_ = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock();
    if (!kristalliModule_.get())
    {
        throw Exception("Fatal: could not get KristalliProtocolModule");
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
            //! \todo Hack, remove and/or find better way: If there is no LoginScreenModule or UIModule,
            /// assume we are running a "dedicated" server, and start the server automatically on default port
            ModuleWeakPtr loginModule = framework_->GetModuleManager()->GetModule("LoginScreen");
            ModuleWeakPtr uiModule = framework_->GetModuleManager()->GetModule("UI");
            if ((!loginModule.lock().get()) && (!uiModule.lock().get()))
            {
                LogInfo("Started server by default");
                server_->Start(cDefaultPort);
            }

            // Load startup scene here (if we have one)
            LoadStartupScene();
            
            check_default_server_start = false;
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
        Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
        if (scene)
            scene->UpdateAttributeInterpolations(frametime);
    }
    
    RESETPROFILER;
}

void TundraLogicModule::LoadStartupScene()
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return;
    
    const boost::program_options::variables_map &options = GetFramework()->ProgramOptions();
    if (options.count("file") == 0)
        return;
    std::string startup_scene = QString(options["file"].as<std::string>().c_str()).trimmed().toStdString();
    if (startup_scene.empty())
        return; // No startup scene specified, ignore.

    // If scene name is expressed as a full path, add it as a recursive asset source for localassetprovider
    boost::filesystem::path scenepath(startup_scene);
    std::string dirname = scenepath.branch_path().string();
    if (!dirname.empty())
    {
        boost::shared_ptr<Asset::LocalAssetProvider> localProvider = GetFramework()->Asset()->GetAssetProvider<Asset::LocalAssetProvider>();
        if (localProvider)
            localProvider->AddStorageDirectory(dirname, "Scene Local", true);
    }
    
    bool useBinary = startup_scene.find(".tbin") != std::string::npos;
    if (!useBinary)
        scene->LoadSceneXML(startup_scene, true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
    else
        scene->LoadSceneBinary(startup_scene, true/*clearScene*/, false/*replaceOnConflict*/, AttributeChange::Default);
}

Console::CommandResult TundraLogicModule::ConsoleStartServer(const StringVector& params)
{
    unsigned short port = cDefaultPort;
    
    try
    {
        if (params.size() > 0)
            port = ParseString<int>(params[0]);
    }
    catch (...) {}
    
    server_->Start(port);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleStopServer(const StringVector& params)
{
    server_->Stop();
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleConnect(const StringVector& params)
{
    if (params.size() < 1)
        return Console::ResultFailure("No address specified");
    
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
    catch (...) {}
    
    client_->Login(params[0], port, username, password);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleDisconnect(const StringVector& params)
{
    client_->Logout(false);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleSaveScene(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene found.");
    if (params.size() < 1)
        return Console::ResultFailure("No filename given.");
    
    bool useBinary = false;
    if ((params.size() > 1) && (params[1] == "binary"))
        useBinary = true;
    
    bool success;
    if (!useBinary)
        success = scene->SaveSceneXML(params[0]);
    else
        success = scene->SaveSceneBinary(params[0]);
    
    if (success)
        return Console::ResultSuccess();
    else
        return Console::ResultFailure("Failed to save the scene.");
}

Console::CommandResult TundraLogicModule::ConsoleLoadScene(const StringVector &params)
{
    ///\todo Add loadScene parameter
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene found.");
    if (params.size() < 1)
        return Console::ResultFailure("No filename given.");
    
    bool useBinary = false;
    if ((params.size() > 1) && (params[1] == "binary"))
        useBinary = true;
    
    QList<Scene::Entity *> entities;
    if (!useBinary)
        entities = scene->LoadSceneXML(params[0], true/*clearScene*/, false/*replaceOnConflcit*/, AttributeChange::Default);
    else
        entities = scene->LoadSceneBinary(params[0], true/*clearScene*/, false/*replaceOnConflcit*/, AttributeChange::Default);
    
    if (!entities.empty())
    {
        return Console::ResultSuccess();
    }
    else
        return Console::ResultFailure("Failed to load the scene.");
}

Console::CommandResult TundraLogicModule::ConsoleImportScene(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene found.");
    if (params.size() < 1)
        return Console::ResultFailure("No filename given.");
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
    QList<Scene::Entity *> entities = importer.Import(filename, dirname, Transform(),
        "local://", AttributeChange::Default, clearscene, replace);
    if (!entities.empty())
    {
        return Console::ResultSuccess();
    }
    else
        return Console::ResultFailure("Failed to import the scene.");
}

Console::CommandResult TundraLogicModule::ConsoleImportMesh(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    if (!scene)
        return Console::ResultFailure("No active scene found.");
    if (params.size() < 1)
        return Console::ResultFailure("No filename given.");
    
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
    Scene::EntityPtr entity = importer.ImportMesh(filename, dirname, Transform(Vector3df(x,y,z),
        Vector3df(xr,yr,zr), Vector3df(xs,ys,zs)), std::string(), "local://", AttributeChange::Default, true);
    
    return Console::ResultSuccess();
}

bool TundraLogicModule::IsServer() const
{
    return kristalliModule_->IsServer();
}

// virtual
bool TundraLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    if (category_id == tundraEventCategory_)
    {
        // Handle login request from the loginwindow
        if (event_id == Events::EVENT_TUNDRA_LOGIN)
        {
            Events::TundraLoginEventData* event_data = checked_static_cast<Events::TundraLoginEventData*>(data);
            if (client_)
                client_->Login(event_data->address_, event_data->port_ ? event_data->port_ : cDefaultPort, event_data->username_, event_data->password_);
        }
    }
    
    if (category_id == kristalliEventCategory_)
    {
        if (client_)
            client_->HandleKristalliEvent(event_id, data);
        if (server_)
            server_->HandleKristalliEvent(event_id, data);
        if (syncManager_)
            syncManager_->HandleKristalliEvent(event_id, data);
    }
    
    return false;
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace TundraLogic;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(TundraLogicModule)
POCO_END_MANIFEST
