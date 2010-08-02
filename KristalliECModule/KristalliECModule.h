/** @file
	@author LudoCraft Oy

	Copyright 2010 LudoCraft Oy.
	All rights reserved.

	@brief
*/
#ifndef incl_KristalliECModule_KristalliECModule_h
#define incl_KristalliECModule_KristalliECModule_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "RexUUID.h"

#include "clb/Network/Network.h"

#include "../KristalliServer/ECServer/MsgComponentUpdated.h"

#include <QObject>

namespace KristalliProtocol
{
    class KristalliProtocolModule;
};

namespace ProtocolUtilities
{
    class WorldStream;
};

class MessageConnection;
struct MsgLoginReply;
struct MsgComponentDeleted;
struct MsgEntityUpdated;
struct MsgEntityDeleted;

namespace KristalliEC
{
    /// Uses KristalliProtocolModule to connect to a Kristalli server. Then proceeds to handle EC replication
    class KristalliECModule : public QObject, public Foundation::ModuleInterface
    {
        static const int MAX_PENDING_EC_UPDATES = 2048;

        Q_OBJECT
        
    public:
        KristalliECModule();
        ~KristalliECModule();

        void Load();
        void Unload();
        void PreInitialize();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS;

        /// @return Module name. Needed for logging.
        static const std::string &NameStatic();

        bool HandleEvent(event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);

        void SubscribeToNetworkEvents();
        void RegisterToComponentChangeSignals(Scene::ScenePtr scene);
        
    public slots:
        //! Trigger EC sync because of component attributes changing
        void OnComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change);
        //! Trigger EC sync because of component added to entity
        void OnComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);
        //! Trigger EC sync because of component removed from entity
        void OnComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change);
        //! Trigger sync of entity removal
        void OnEntityRemoved(Scene::Entity* entity, AttributeChange::Type change);
        
    private:
        KristalliProtocol::KristalliProtocolModule* GetProtocolModule() const;
        MessageConnection* GetConnection() const;
        void HandlePendingConnection();
        void HandleKristalliMessage(message_id_t id, const char* data, size_t numBytes);
        void HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);
        void HandleLoginReplyMessage(MsgLoginReply& msg);
        void HandleComponentUpdatedMessage(MsgComponentUpdated& msg);
        void HandleComponentDeletedMessage(MsgComponentDeleted& msg);
        void HandleEntityUpdatedMessage(MsgEntityUpdated& msg);
        void HandleEntityDeletedMessage(MsgEntityDeleted& msg);
        u32 GetNextActionID();
        Scene::EntityPtr LookUpEntity(entity_id_t id, const RexUUID& fullid);
        RexUUID GetEntityFullID(Scene::Entity* entity);
        void HandlePendingComponentUpdates();
        
        /// Connection (and login) to Kristalli EC server pending
        bool connectPending_;
        /// Login to Kristalli EC server established. Need to get this before doing anything
        bool loginOk_;
        /// Should update pending component updates on next frame
        bool updatePendingUpdates_;
        
        /// Event manager.
        Foundation::EventManagerPtr eventManager_ ;

        /// Id for "Scene" event category.
        event_category_id_t sceneEventCategory_;
        
        /// Id for "Kristalli" event category.
        event_category_id_t kristalliEventCategory_;
        
        /// Id for "NetworkIn" event category.
        event_category_id_t networkInEventCategory_;

        /// Id for "NetworkState" event category.
        event_category_id_t networkStateEventCategory_;
        
        /// Id for "Framework" event category.
        event_category_id_t frameworkEventCategory_;
        
        /// Current world stream
        boost::shared_ptr<ProtocolUtilities::WorldStream> worldStream_;
        
        /// Pending prim-related component updates, that are waiting for the prim to appear via opensim
        std::vector<MsgComponentUpdated> pendingComponentUpdates_;
        
        /// Running action ID
        u32 actionID_;
    };
}

#endif

