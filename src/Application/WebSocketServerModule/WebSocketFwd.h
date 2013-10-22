
#pragma once

#include "CoreTypes.h"

#include <QHash>
#include <QObject>
#include <QVariant>
#include <QString>
#include <QStringList>

#include <vector>

class WebSocketServerModule;

namespace WebSocket
{
    class Server;
    class Handler;
    class UserConnection;
    class SyncManager;
    
    typedef std::vector<WebSocket::UserConnection*> UserConnectionList;
    typedef QVariantHash LoginPropertyMap;
}

class Framework;

struct UserConnectedResponseData;
struct libwebsocket_context;

typedef shared_ptr<WebSocket::Server> WebSocketServerPtr;
typedef shared_ptr<WebSocket::SyncManager> WebSocketSyncManagerPtr;
typedef shared_ptr<WebSocket::UserConnection> WebSocketUserConnectionPtr;
