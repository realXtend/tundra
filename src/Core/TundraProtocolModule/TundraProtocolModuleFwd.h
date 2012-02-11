/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   TundraProtocolModuleFwd.h
    @brief  Forward declarations and type defines for commonly used TundraProtocolModule plugin classes. */

#pragma once

#include <kNetFwd.h>

#include <boost/smart_ptr.hpp>
#include <list>

class KristalliProtocolModule;

namespace TundraLogic
{
    class TundraLogicModule;
    class Client;
    class Server;
    class SyncManager;
}

class UserConnection;
typedef boost::shared_ptr<UserConnection> UserConnectionPtr;
typedef boost::weak_ptr<UserConnection> UserConnectionWeakPtr;
typedef std::list<UserConnectionPtr> UserConnectionList;

struct SceneSyncState;
struct EntitySyncState;
struct ComponentSyncState;
struct UserConnectedResponseData;

struct MsgLogin;
struct MsgLoginReply;
struct MsgClientJoined;
struct MsgClientLeft;
struct MsgAssetDiscovery;
struct MsgAssetDeleted;
struct MsgEntityAction;
