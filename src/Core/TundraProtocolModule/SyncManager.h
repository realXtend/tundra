// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleFwd.h"
#include "TundraProtocolModuleApi.h"

#include "SyncState.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "EntityAction.h"

#include <kNetFwd.h>
#include <kNet/Types.h>

#include <QObject>

class Framework;

namespace TundraLogic
{
/// Performs synchronization of the changes in a scene between the server and the client.
/** @todo Interest management functionality description.

    Alternatively, SyncManager and SceneSyncState can be used to implement prioritization logic on how and when
    a sync state is filled per client connection. */
class TUNDRAPROTOCOL_MODULE_API SyncManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float updatePeriod READ GetUpdatePeriod WRITE SetUpdatePeriod) /**< @copydoc updatePeriod_ */
    Q_PROPERTY(bool interestManagementEnabled READ IsInterestManagementEnabled WRITE SetInterestManagementEnabled) /**< @copydoc interestManagementEnabled */
    Q_PROPERTY(EntityPtr observer READ Observer WRITE SetObserver) /**< @copydoc observer */

public:
    explicit SyncManager(TundraLogicModule* owner);
    ~SyncManager();
    
    /// Register to entity/component change signals from a specific scene and start syncing them
    void RegisterToScene(ScenePtr scene);
    
    /// Accumulate time & send pending sync messages if enough time passed from last update
    void Update(f64 frametime);
    
    /// Create new replication state for user and dirty it (server operation only)
    void NewUserConnected(const UserConnectionPtr &user);

    /// Enables or disables the interest management. @remark Interest management
    void SetInterestManagementEnabled(bool enabled) { interestManagementEnabled = enabled; }
    /// Returns is the interest management enabled. @remark Interest management
    bool IsInterestManagementEnabled() const { return interestManagementEnabled; }

    /// Sets the client's observer entity. @remark Interest management
    /** @note The entity needs to have Placeable component present in order to be usable. */
    void SetObserver(const EntityPtr &entity) { observer = entity; }
    /// Returns the observer entity, if any. @remark Interest management
    EntityPtr Observer() const { return observer.lock(); }

public slots:
    /// Set update period (seconds), 0.01 at fastest.
    void SetUpdatePeriod(float period);

    /// Get update period
    float GetUpdatePeriod() const { return updatePeriod_; }

    /// Returns SceneSyncState for a client connection.
    /** @note This slot is only usable when running as server, otherwise will return null ptr.
        @param u32 connection ID of the client. */
    SceneSyncState* SceneState(u32 connectionId) const;
    SceneSyncState* SceneState(const UserConnectionPtr &connection) const; /**< @overload @param connection Client connection.*/

signals:
    /// This signal is emitted when a new user connects and a new SceneSyncState is created for the connection.
    /// @note See signals of the SceneSyncState object to build prioritization logic how the sync state is filled.
    /// @remark Enables a 'pending' logic in SyncManager, with which a script can throttle the sending of entities to clients.
    void SceneStateCreated(UserConnection *user, SceneSyncState *state);
    
private slots:
    /// Network message received from an user connection
    void HandleNetworkMessage(UserConnection* user, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes);

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

    /// Trigger sync of entity parent change
    void OnEntityParentChanged(Entity* entity, Entity* newParent, AttributeChange::Type change);
    
    /// Trigger sync of a custom component type
    void OnPlaceholderComponentTypeRegistered(u32 typeId, const QString& typeName, AttributeChange::Type change);

private:
    /// Craft a component full update, with all static and dynamic attributes.
    void WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp);
    /// Handle entity action message.
    void HandleEntityAction(UserConnection* source, MsgEntityAction& msg);
    /// Handle create entity message.
    void HandleCreateEntity(UserConnection* source, const char* data, size_t numBytes);
    /// Handle create components message.
    void HandleCreateComponents(UserConnection* source, const char* data, size_t numBytes);
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
    /// Handle component type registration message.
    void HandleRegisterComponentType(UserConnection* source, const char* data, size_t numBytes);
    /// Handle entity parent change message.
    void HandleSetEntityParent(UserConnection* source, const char* data, size_t numBytes);

    void HandleRigidBodyChanges(UserConnection* source, kNet::packet_id_t packetId, const char* data, size_t numBytes);
    
    void ReplicateRigidBodyChanges(UserConnection* user);

    void InterpolateRigidBodies(f64 frametime, SceneSyncState* state);

    void ReplicateComponentType(u32 typeId, UserConnection* connection = 0);

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

    /// @remark Interest management.
    void HandleObserverPosition(UserConnection* source, const char* data, size_t numBytes);
    /// Sends client's observer information. @remark Interest management
    void SendObserverPosition(UserConnection* connection, SceneSyncState *senderState);
    /// (Re)computes priority for entity. @remark Interest management
    void ComputePriorityForEntitySyncState(SceneSyncState *sceneState, EntitySyncState *entityState, Entity *entity) const;
    /// (Re)computes priorities for all entities in the scene. @remark Interest management
    void ComputePrioritiesForEntitySyncStates(SceneSyncState *sceneState) const;

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

    /// The sender of a component type. Used to avoid sending component description back to sender
    UserConnection* componentTypeSender_;

    /// Set of custom component type id's that were received from the server, to avoid echoing them back in ProcessSyncState
    std::set<u32> componentTypesFromServer_;

    float prioUpdateAcc_; /**< Time accumulator for priority update @remark Interest management */
    /// Is interest management enabled.
    /** On client this means that the observer's position information is sent to the server.
        On server this means that dirty entities are sorted and synced according to their priority that is calculated according to observer position.
        @remark Interest management */
    bool interestManagementEnabled;
    /// If interestManagementEnabled is true, on client this entity's position information is sent to the server.
    /** @remark Interest management */
    EntityWeakPtr observer;
};

}
