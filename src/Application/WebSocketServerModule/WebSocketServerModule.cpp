
#include "WebSocketServerModule.h"

#include "WebSocketServer.h"
#include "WebSocketSyncManager.h"
#include "WebSocketScriptTypeDefines.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "EntityAction.h"
#include "LoggingFunctions.h"

#include "SceneAPI.h"
#include "Scene.h"

WebSocketServerModule::WebSocketServerModule() :
    IModule("WebSocketServerModule"),
    LC("[WebSocketServerModule]: ")
{
}

WebSocketServerModule::~WebSocketServerModule()
{
    StopServer();
}

void WebSocketServerModule::Load()
{
    isServer_ = framework_->HasCommandLineParameter("--server");
}

void WebSocketServerModule::Initialize()
{
    if (isServer_)
        StartServer();
}

void WebSocketServerModule::Uninitialize()
{
    StopServer();
}

void WebSocketServerModule::Update(f64 frametime)
{
    if (!isServer_)
        return;
    
    if (server_.get())
        server_->Update(frametime);
    if (syncManager_.get())
        syncManager_->Update(frametime);
}

bool WebSocketServerModule::IsServer()
{
    return isServer_;
}

const WebSocketSyncManagerPtr& WebSocketServerModule::GetSyncManager()
{
    return syncManager_;
}

const WebSocketServerPtr& WebSocketServerModule::GetServer()
{
    return server_;
}

void WebSocketServerModule::StartServer()
{
    if (!isServer_)
        return;
    if (server_.get())
    {
        LogWarning(LC + "Server already started.");
        return;
    }

    // Server
    server_ = WebSocketServerPtr(new WebSocket::Server(framework_));
    server_->Start();

    // Sync Manager
    syncManager_ = WebSocketSyncManagerPtr(new WebSocket::SyncManager(this));
    
    // Use or wait for the server scene registered by TundraProtocolModule
    ScenePtr scene = framework_->Scene()->GetScene("TundraServer");
    if (scene.get())
        syncManager_->RegisterToScene(scene);
    else
        connect(framework_->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString &)), Qt::UniqueConnection);

    // Connect new users for scene state creation in syncmanager.
    connect(server_.get(), SIGNAL(UserConnected(WebSocket::UserConnection*, QVariantMap*)),
            syncManager_.get(), SLOT(OnUserConnected(WebSocket::UserConnection*, QVariantMap*)));
    connect(server_.get(), SIGNAL(NetworkMessageReceived(WebSocket::UserConnection*, kNet::message_id_t, const char*, size_t)),
            syncManager_.get(), SLOT(OnNetworkMessageReceived(WebSocket::UserConnection*, kNet::message_id_t, const char*, size_t)));
    //connect(server_.get(), SIGNAL(ClientEntityAction(WebSocket::UserConnection*, MsgEntityAction)),
    //        syncManager_.get(), SLOT(OnClientEntityAction(WebSocket::UserConnection*, MsgEntityAction)));

    framework_->RegisterDynamicObject("websocketserver", server_.get());
    framework_->RegisterDynamicObject("websocketsyncmanager", syncManager_.get());
                
    emit ServerStarted(server_);
}

void WebSocketServerModule::StopServer()
{
    if (server_.get())
        server_->Stop();
    server_.reset();
}

void WebSocketServerModule::OnSceneAdded(const QString &sceneName)
{
    if (!syncManager_.get() || syncManager_->GetRegisteredScene().get())
        return;
    if (sceneName == "TundraServer")
        syncManager_->RegisterToScene(framework_->Scene()->GetScene(sceneName));
}

void WebSocketServerModule::OnScriptEngineCreated(QScriptEngine *engine)
{
    RegisterWebSocketPluginMetaTypes(engine);
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new WebSocketServerModule();
        fw->RegisterModule(module);
    }
}
