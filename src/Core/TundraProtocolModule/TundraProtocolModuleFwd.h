/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   TundraProtocolModuleFwd.h
    @brief  Forward declarations and type defines for commonly used TundraProtocolModule plugin classes. */

#pragma once

#include "CoreTypes.h"

#include <kNetFwd.h>

#include <QVariantMap>

class KristalliProtocolModule;

namespace TundraLogic
{
    class TundraLogicModule;
    class Client;
    class Server;
    class SyncManager;
}

using TundraLogic::TundraLogicModule;

class UserConnection;
class KNetUserConnection;
typedef shared_ptr<UserConnection> UserConnectionPtr;
typedef shared_ptr<KNetUserConnection> KNetUserConnectionPtr;
typedef weak_ptr<UserConnection> UserConnectionWeakPtr;
typedef std::list<UserConnectionPtr> UserConnectionList;

class SceneSyncState;
struct EntitySyncState;
struct ComponentSyncState;
typedef std::map<entity_id_t, EntitySyncState> EntitySyncStateMap;
struct UserConnectedResponseData;
class EntityPrioritizer;

typedef QVariantMap LoginPropertyMap; ///< propertyName-propertyValue map of login properties.

struct MsgLogin;
struct MsgLoginReply;
struct MsgClientJoined;
struct MsgClientLeft;
struct MsgAssetDiscovery;
struct MsgAssetDeleted;
struct MsgEntityAction;
