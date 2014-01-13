// For conditions of distribution and use, see copyright notice in LICENSE

#include "WebSocketServerModule.h"

#include "WebSocketServer.h"

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
}

bool WebSocketServerModule::IsServer()
{
    return isServer_;
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

    framework_->RegisterDynamicObject("websocketserver", server_.get());

    emit ServerStarted(server_);
}

void WebSocketServerModule::StopServer()
{
    if (server_.get())
        server_->Stop();
    server_.reset();
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
