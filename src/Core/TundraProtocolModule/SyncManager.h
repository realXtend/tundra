// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleFwd.h"
#include "TundraProtocolModuleApi.h"

#include "SyncState.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "EntityAction.h"
#include "InterestManager.h"
#include "HighPerfClock.h"

#include <kNetFwd.h>
#include <kNet/Types.h>

#include <QObject>

class Framework;

namespace TundraLogic
{
/// Performs synchronization of the changes in a scene between the server and the client.
/** SyncManager and SceneSyncState combined can be used to implement prioritization logic on how and when
    a sync state is filled per client connection. SyncManager object is only exposed to scripting on the server. */
class TUNDRAPROTOCOL_MODULE_API SyncManager : public QObject
{
    Q_OBJECT

public:
    explicit SyncManager(TundraLogicModule* owner);
    ~SyncManager();
    
    /// Register to entity/component change signals from a specific scene and start syncing them
    void RegisterToScene(ScenePtr scene);
    
    /// Accumulate time & send pending sync messages if enough time passed from last update
    void Update(f64 frametime);
    
    /// Create new replication state for user and dirty it (server operation only)
    void NewUserConnected(const UserConnectionPtr &user);

    /// Get and Set the IM
    InterestManager* GetInterestManager();

    void SetInterestManager(InterestManager* im);

public slots:
    /// Set update period (seconds)
    void SetUpdatePeriod(float period);

    /// Get update period
    float GetUpdatePeriod() const { return updatePeriod_; }

    /// Returns SceneSyncState for a client connection.
    /** @note This slot is only exposed on Server, other wise will return null ptr.
        @param u32 connection ID of the client. */
    SceneSyncState* SceneState(u32 connectionId) const;
    SceneSyncState* SceneState(const UserConnectionPtr &connection) const; /**< @overload @param connection Client connection.*/

    /// Upates Interest Manager settings.
    /** @param enabled If true, the IM scheme is allowed to filter traffic.
        @param bool eucl If true, the euclidean distance filter is active.
        @param bool ray If true, the ray visibility filter is active.
        @param bool rel If true, the relevance filter is active.
        @param int critrange specifies the radius for the critical area.
        @param int rayrange specifies the radius for the raycasting.
        @param int relrange specifies the radius for the relevance filtering.
        @param int updateint specifies the update interval for the relevance filtering.
        @param int raycastint specifies the raycasting interval for the ray visibility filter. */
    void UpdateInterestManagerSettings(bool enabled, bool eucl, bool ray, bool rel, int critrange, int relrange, int updateint, int raycastint);

    void SendCameraUpdateRequest(UserConnectionPtr conn, bool enabled);

signals:
    /// This signal is emitted when a new user connects and a new SceneSyncState is created for the connection.
    /// @note See signals of the SceneSyncState object to build prioritization logic how the sync state is filled.
    void SceneStateCreated(UserConnection *user, SceneSyncState *state);
    
private slots:
    /// Network message received from an user connection
    void HandleNetworkMessage(kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes);
    /// Handle client connecting to the server.
    void HandleClientConnected(UserConnectedResponseData*);
    /// Handle client disconnecting from the server.
    void HandleClientDisconnected();

    /// Trigger EC sync because of component attributes changing
    void OnAttributeChanged(IComponent* comp, IAttribute* attr, AttributeChange::Type change);

    /// Trigger EC sync because of component attribute added
    void OnAttributeAdded(IComponent* comp, IAttribute* attr, AttributeChange::Type change);

    /// Trigger EC sync because of component attribute removed
    void OnAttributeRemoved(IComponent* comp, IAttribute* attr, AttributeChange::Type change);
    
    /// Trigger EC sync because of component added to entity
    void OnComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change);
    
    /// Trigger EC sync because of component removed from entity
    void OnComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change);
    
    /// Trigger sync of entity creation
    void OnEntityCreated(Entity* entity, AttributeChange::Type change);
    
    /// Trigger sync of entity removal
    void OnEntityRemoved(Entity* entity, AttributeChange::Type change);

    /// Trigger sync of entity action.
    void OnActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type);

    /// Trigger sync of entity action to specific user
    void OnUserActionTriggered(UserConnection* user, Entity *entity, const QString &action, const QStringList &params);

    /// Trigger sync of entity properties (temporary status) change
    void OnEntityPropertiesChanged(Entity* entity, AttributeChange::Type change);
    
private:
    /// Craft a component full update, with all static and dynamic attributes.
    void WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp);
    /// Handle entity action message.
    void HandleEntityAction(UserConnection* source, MsgEntityAction& msg);
    /// Handle create entity message.
    void HandleCreateEntity(UserConnection* source, const char* data, size_t numBytes);
    /// Handle create components message.
    void HandleCreateComponents(UserConnection* source, const char* data, size_t numBytes);
    /// Handle a Camera Orientation Update message
    void HandleCameraOrientation(UserConnection* source, const char* data, size_t numBytes);
    /// Handle create attributes message.
    void HandleCreateAttributes(UserConnection* source, const char* data, size_t numBytes);
    /// Handle edit attributes message.
    void HandleEditAttributes(UserConnection* source, const char* data, size_t numBytes);
    /// Handle remove attributes message.
    void HandleRemoveAttributes(UserConnection* source, const char* data, size_t numBytes);
    /// Handle remove components message.
    void HandleRemoveComponents(UserConnection* source, const char* data, size_t numBytes);
    /// Handle remove entities message.
    void HandleRemoveEntity(UserConnection* source, const char* data, size_t numBytes);
    /// Handle create entity reply message.
    void HandleCreateEntityReply(UserConnection* source, const char* data, size_t numBytes);
    /// Handle create components reply message.
    void HandleCreateComponentsReply(UserConnection* source, const char* data, size_t numBytes);
    /// Handle entity properties change message.
    void HandleEditEntityProperties(UserConnection* source, const char* data, size_t numBytes);

    void HandleRigidBodyChanges(UserConnection* source, kNet::packet_id_t packetId, const char* data, size_t numBytes);
    
    void ReplicateRigidBodyChanges(UserConnection* user);

    void InterpolateRigidBodies(f64 frametime, SceneSyncState* state);

    /// Read client extrapolation time parameter from command line and match it to the current sync period.
    void GetClientExtrapolationTime();

    /// Process one user connection's sync state for changes in the scene. Note that on the client the server is a "virtual" user
    /** @param user User connection to process */
    void ProcessSyncState(UserConnection* user);
    
    /// Validate the scene manipulation action. If returns false, it is ignored
    /** @param source Where the action came from
        @param messageID Network message id
        @param entityID What entity it affects */
    bool ValidateAction(UserConnection* source, unsigned messageID, entity_id_t entityID);
    
    ScenePtr GetRegisteredScene() const { return scene_.lock(); }

    /// Owning module
    TundraLogicModule* owner_;
    
    /// Framework pointer
    Framework* framework_;
    
    /// Scene pointer
    SceneWeakPtr scene_;
    
    /// Time period for update, default 1/30th of a second
    float updatePeriod_;
    /// Time accumulator for update
    float updateAcc_;
    
    /// Physics client interpolation/extrapolation period length as number of network update intervals (default 3)
    float maxLinExtrapTime_;
    /// Disable client physics handoff -flag
    bool noClientPhysicsHandoff_;
    
    /// "User" representing the server connection (client only)
    UserConnectionPtr serverConnection_;
    
    /// Fixed buffers for crafting messages
    char createEntityBuffer_[64 * 1024];
    char createCompsBuffer_[64 * 1024];
    char editAttrsBuffer_[64 * 1024];
    char createAttrsBuffer_[16 * 1024];
    char attrDataBuffer_[16 * 1024];
    char removeCompsBuffer_[1024];
    char removeEntityBuffer_[1024];
    char removeAttrsBuffer_[1024];
    std::vector<u8> changedAttributes_;

    /// Interest manager currently in use, null if none
    InterestManager *interestmanager_;
};

}
