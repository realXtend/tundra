// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SyncManager_h
#define incl_TundraLogicModule_SyncManager_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "SceneManager.h"

#include <QObject>
#include <map>
#include <set>

struct MsgCreateEntity;
struct MsgRemoveEntity;
struct MsgUpdateComponents;
struct MsgRemoveComponents;
struct MsgEntityIDCollision;

class QDomDocument;

namespace KristalliProtocol
{
    struct UserConnection;
}

class MessageConnection;

namespace TundraLogic
{

class TundraLogicModule;

struct RemovedComponent
{
    QString typename_;
    QString name_;
};

class SyncManager : public QObject
{
    Q_OBJECT
    
public:
    //! Constructor
    SyncManager(TundraLogicModule* owner, Foundation::Framework* fw);
    
    //! Destructor
    ~SyncManager();
    
    //! Register to entity/component change signals and start syncing them
    void RegisterToScene(Scene::ScenePtr scene);
    
    //! Send pending sync messages for one frame
    void Update();
    
    //! Replicate the whole scene to a new user
    void NewUserConnected(KristalliProtocol::UserConnection* user);
    
    //! Handle create entity message
    void HandleCreateEntity(MessageConnection* source, const MsgCreateEntity& msg);
    //! Handle remove entity message
    void HandleRemoveEntity(MessageConnection* source, const MsgRemoveEntity& msg);
    //! Handle update components message
    void HandleUpdateComponents(MessageConnection* source, const MsgUpdateComponents& msg);
    //! Handle remove components message
    void HandleRemoveComponents(MessageConnection* source, const MsgRemoveComponents& msg);
    //! Handle entityID collision message
    void HandleEntityIDCollision(MessageConnection* source, const MsgEntityIDCollision& msg);
    
private slots:
    //! Trigger EC sync because of component attributes changing
    void OnComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change);
    //! Trigger EC sync because of component added to entity
    void OnComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);
    //! Trigger EC sync because of component removed from entity
    void OnComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);
    //! Trigger sync of entity creation
    void OnEntityCreated(Scene::Entity* entity, AttributeChange::Type change);
    //! Trigger sync of entity removal
    void OnEntityRemoved(Scene::Entity* entity, AttributeChange::Type change);
    
private:
    //! Return the messageconnections to where we should send each sync message. For client, this is the server. For server, this is all connected clients
    /*! \todo In the future, we may need to send different entities/components to different users, based on interest management
     */
    std::vector<MessageConnection*> GetConnectionsToSyncTo();
    
    //! Validate the scene manipulation action. If returns false, it is ignored
    /*! \param source Where the action came from
        \param messageID Network message id
        \param entityID What entity it affects
     */
    bool ValidateAction(MessageConnection* source, unsigned messageID, entity_id_t entityID);
    
    //! Send serializable components of an entity to a connection, using either a CreateEntity or UpdateComponents packet
    /*! \param connections MessageConnection(s) to use
        \param entity Entity
        \param createEntity Whether to use a CreateEntity packet. If false, use a UpdateComponents packet instead
        \param allComponents Whether to send all components, or only those that are dirty
        Note: This will not reset any changeflags in the components or attributes!
     */
    void SerializeAndSendComponents(const std::vector<MessageConnection*>& connections, Scene::EntityPtr entity, bool createEntity = false, bool allComponents = false);
    
    //! Owning module
    TundraLogicModule* owner_;
    
    //! Framework pointer
    Foundation::Framework* framework_;
    
    //! Entities created within the frame
    std::set<entity_id_t> createdEntities_;
    
    //! Locally dirty entities, to be replicated.
    std::set<entity_id_t> dirtyEntities_;
    
    //! Removed entities
    std::set<entity_id_t> removedEntities_;
    
    //! Components that have been removed from specific entities on the frame
    std::map<entity_id_t, std::vector<RemovedComponent> > removedComponents_;
};

}

#endif

