// For conditions of distribution and use, see copyright notice in LICENSE
// 
#pragma once

#include "WebSocketServerModuleApi.h"
#include "WebSocketFwd.h"

#include "IModule.h"
#include "CoreTypes.h"

#include <QString>

/// WebSocketServerModule
/** This module was originally developed by Adminotech Ltd. for the Meshmoon hosting platform.
    The code was open sourced at 22.10.2013 with the Tundra license to the realXtend
    Tundra repository. Open sourcing was done to bring WebSocket connectivity to the core Tundra
    platform for everyone, and in hopes of that this module will be helpful to the open source
    realXtend ecosystem and its users.
    
    The module will hopefully be developed further as open source for the common good.
*/
class WEBSOCKET_SERVER_MODULE_API WebSocketServerModule : public IModule
{
Q_OBJECT

public:
    WebSocketServerModule();
    virtual ~WebSocketServerModule();

    void Load();
    void Initialize();
    void Uninitialize();
    
    void Update(f64 frametime);
    
public slots:
    bool IsServer();
    
    const WebSocketSyncManagerPtr& GetSyncManager();
    const WebSocketServerPtr& GetServer();
    
signals:
    void ServerStarted(WebSocketServerPtr server);
    
private slots:
    void StartServer();
    void StopServer();
    
    void OnSceneAdded(const QString &sceneName);

private:
    QString LC;

    bool isServer_;
    
    WebSocketSyncManagerPtr syncManager_;
    WebSocketServerPtr server_;
};
