/** @file
    @author LudoCraft Oy

    Copyright 2009 LudoCraft Oy.
    All rights reserved.

    @brief
*/
#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "KristalliECModule.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "Profiler.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "NetworkEvents.h"
#include "WorldStream.h"
#include "RexTypes.h"
#include "SceneEvents.h"
#include "ComponentManager.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "WorldLogicInterface.h"
#include "EC_OpenSimPrim.h"

#include "../KristalliServer/KristalliServer/KristalliServerMessages.h"
#include "../KristalliServer/KristalliServer/MsgLogin.h"
#include "../KristalliServer/KristalliServer/MsgLoginReply.h"

#include "../KristalliServer/ECServer/ECServerMessages.h"
#include "../KristalliServer/ECServer/MsgModifyComponent.h"
#include "../KristalliServer/ECServer/MsgDeleteComponent.h"
#include "../KristalliServer/ECServer/MsgComponentUpdated.h"
#include "../KristalliServer/ECServer/MsgComponentDeleted.h"
#include "../KristalliServer/ECServer/MsgEntityUpdated.h"
#include "../KristalliServer/ECServer/MsgEntityDeleted.h"
#include "../KristalliServer/ECServer/MsgCreateEntity.h"
#include "../KristalliServer/ECServer/MsgDeleteEntity.h"

#include <QDomDocument>

using namespace KristalliProtocol;

namespace KristalliEC
{

namespace
{
    const std::string moduleName("KristalliECModule");
}

std::string BufferToString(const std::vector<s8>& buffer)
{
    if (buffer.size())
        return std::string((const char*)&buffer[0], buffer.size());
    else
        return std::string();
}

std::vector<s8> StringToBuffer(const std::string& str)
{
    std::vector<s8> ret;
    ret.resize(str.size());
    if (str.size())
        memcpy(&ret[0], &str[0], str.size());
    return ret;
}

KristalliECModule::KristalliECModule() :
    ModuleInterface(NameStatic()),
    connectPending_(false),
    loginOk_(false),
    actionID_(0)
{
}

KristalliECModule::~KristalliECModule()
{
}

void KristalliECModule::Load()
{
}

void KristalliECModule::Unload()
{
}

void KristalliECModule::PreInitialize()
{
}

void KristalliECModule::Initialize()
{
}

void KristalliECModule::PostInitialize()
{
    eventManager_ = framework_->GetEventManager();

    frameworkEventCategory_ = eventManager_->QueryEventCategory("Framework");
    kristalliEventCategory_ = eventManager_->QueryEventCategory("Kristalli");
    sceneEventCategory_ = eventManager_->QueryEventCategory("Scene");
}

void KristalliECModule::Uninitialize()
{
}

void KristalliECModule::Update(f64 frametime)
{
    RESETPROFILER;
    
    if (connectPending_)
        HandlePendingConnection();
        
    if (updatePendingUpdates_)
    {
        updatePendingUpdates_ = false;
        if (pendingComponentUpdates_.size())
            HandlePendingComponentUpdates();
    }
}

const std::string &KristalliECModule::NameStatic()
{
    return moduleName;
}

KristalliProtocolModule* KristalliECModule::GetProtocolModule() const
{
    return framework_->GetModuleManager()->GetModule<KristalliProtocolModule>().lock().get();
}

MessageConnection* KristalliECModule::GetConnection() const
{
    KristalliProtocolModule* module = GetProtocolModule();
    if (module)
        return module->GetMessageConnection();
    else
        return 0;
}

void KristalliECModule::HandlePendingConnection()
{
    MessageConnection* connection = GetConnection();
    if (!connection)
    {
        LogError("No connection was created");
        connectPending_ = false;
        return;
    }
    
    if (connection->GetConnectionState() == ConnectionPending)
        return;
        
    connectPending_ = false;
    
    if (connection->GetConnectionState() != ConnectionOK)
    {
        LogError("Failed to connect to KristalliServer!");
        return;
    }
    
    connection->SetDatagramInFlowRatePerSecond(200);
    
    // Kristalli connection is established, now send a login message
    //! \todo Is this the right place for this? The same connection is also used for voip
    LogInfo("KristalliServer connection established, sending login message");
    if (!worldStream_)
    {
        LogError("No worldstream, cannot proceed with KristalliServer login");
        return;
    }
    
    ProtocolUtilities::ClientParameters& clientParams = worldStream_->GetInfo();
    
    MsgLogin msg;
    for (unsigned i = 0; i < RexUUID::cSizeBytes; ++i)
        msg.userUUID[i] = clientParams.agentID.data[i];
    msg.userName = StringToBuffer(worldStream_->GetUsername());
    connection->Send(msg);
}

void KristalliECModule::SubscribeToNetworkEvents()
{
    networkInEventCategory_ = eventManager_->QueryEventCategory("NetworkIn");
    networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
}

void KristalliECModule::RegisterToComponentChangeSignals(Scene::ScenePtr scene)
{
    connect(scene.get(), SIGNAL( ComponentChanged(Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentChanged(Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( EntityRemoved(Scene::Entity*, AttributeChange::Type) ),
        this, SLOT( OnEntityRemoved(Scene::Entity*, AttributeChange::Type) ));
}

void KristalliECModule::OnComponentChanged(Foundation::ComponentInterface* component, AttributeChange::Type change)
{
    MessageConnection* connection = GetConnection();
    if ((!loginOk_) || (!connection))
        return;
    
    if (change != AttributeChange::Local)
    {
        // Change that came from network: simply acknowledge it
        component->ResetChange();
        return;
    }
    
    // Local change: should be serialized & sent
    Scene::Entity* parent_entity = component->GetParentEntity();
    if (parent_entity)
    {
        if (component->IsSerializable())
        {
            QDomDocument temp_doc;
            QDomElement entity_elem = temp_doc.createElement("entity");
            component->SerializeTo(temp_doc, entity_elem);
            temp_doc.appendChild(entity_elem);
            QByteArray bytes = temp_doc.toByteArray();

            MsgModifyComponent msg;
            msg.actionID = GetNextActionID();
            msg.entityID = parent_entity->GetId();
            memcpy(msg.entityUUID, GetEntityFullID(parent_entity).data, 16);
            msg.componentTypeName = StringToBuffer(component->TypeName());
            msg.componentName = StringToBuffer(component->Name());
            msg.componentData.resize(bytes.size());
            memcpy(&msg.componentData[0], bytes.data(), bytes.size());
            
            connection->Send(msg);
        }
    }
    
    // Acknowledge: reset dirtyflags
    component->ResetChange();
}

void KristalliECModule::OnComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* component, AttributeChange::Type change)
{
    // Can actually reuse the OnComponentChanged code, because the protocol does not contain separate message for just adding a component
    //! \todo: when we serialize per attribute, we have to make sure all are dirtied now, so that initial state is sent
    OnComponentChanged(component, change);
    
    updatePendingUpdates_ = true;
}

void KristalliECModule::OnComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* component, AttributeChange::Type change)
{
    MessageConnection* connection = GetConnection();
    if ((!loginOk_) || (!connection))
        return;
    
    if (change != AttributeChange::Local)
        return;
    
    MsgDeleteComponent msg;
    msg.actionID = GetNextActionID();
    msg.entityID = entity->GetId();
    memcpy(msg.entityUUID, GetEntityFullID(entity).data, 16);
    msg.componentTypeName = StringToBuffer(component->TypeName());
    msg.componentName = StringToBuffer(component->Name());

    connection->Send(msg);
}

void KristalliECModule::OnEntityRemoved(Scene::Entity* entity, AttributeChange::Type change)
{
    MessageConnection* connection = GetConnection();
    if ((!loginOk_) || (!connection))
        return;
    
    if (change != AttributeChange::Local)
        return;
    
    MsgDeleteEntity msg;
    msg.actionID = GetNextActionID();
    msg.entityID = entity->GetId();
    memcpy(msg.entityUUID, GetEntityFullID(entity).data, 16);
    
    connection->Send(msg);
}

bool KristalliECModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    PROFILE(KristalliProtocolModule_HandleEvent);

    if (category_id == networkStateEventCategory_)
    {
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            KristalliProtocolModule* module = GetProtocolModule();
            if (module)
                module->Disconnect();
            else
                LogError("Could not get KristalliProtocolModule for disconnecting");
                
            pendingComponentUpdates_.clear();
            connectPending_ = false;
            loginOk_ = false;
        }
    }
    
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            // Begin to listen network events.
            SubscribeToNetworkEvents();
        }

        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            // As simulator is connected, connect also to Kristalli
            ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
            {
                std::vector<std::string> address;
                worldStream_ = event_data->WorldStream;
                boost::split(address, event_data->WorldStream->GetInfo().gridUrl, boost::is_any_of(":"));
                if (address.size() != 2)
                {
                    LogWarning("Could not parse connection grid url! Expected IP:PORT format!");
                    return false;
                }

                unsigned short uPort = 2345;
                KristalliProtocolModule* module = GetProtocolModule();
                if (module)
                {
                    LogInfo("Connecting to KristalliServer " + address[0]);
                    module->Connect(address[0].c_str(), uPort);
                    connectPending_ = true;
                    loginOk_ = false;
                }
                else
                    LogError("Could not get KristalliProtocolModule for connecting");
            }
        }
    }

    if (category_id == kristalliEventCategory_)
    {
        if (event_id == Events::NETMESSAGE_IN)
        {
            Events::KristalliNetMessageIn* eventData = checked_static_cast<Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->id, eventData->data, eventData->numBytes);
        }
    }
    
    if (category_id == sceneEventCategory_)
    {
        HandleSceneEvent(event_id, data);
    }
    
    return false;
}

void KristalliECModule::HandleKristalliMessage(message_id_t id, const char* data, size_t numBytes)
{
    switch (id)
    {
    case cLoginReplyMessage:
        {
            MsgLoginReply msg(data, numBytes);
            HandleLoginReplyMessage(msg);
        }
        break;
    case cComponentUpdatedMessage:
        {
            MsgComponentUpdated msg(data, numBytes);
            HandleComponentUpdatedMessage(msg);
        }
        break;
    case cComponentDeletedMessage:
        {
            MsgComponentDeleted msg(data, numBytes);
            HandleComponentDeletedMessage(msg);
        }
        break;
    case cEntityUpdatedMessage:
        {
            MsgEntityUpdated msg(data, numBytes);
            HandleEntityUpdatedMessage(msg);
        }
        break;
    case cEntityDeletedMessage:
        {
            MsgEntityDeleted msg(data, numBytes);
            HandleEntityDeletedMessage(msg);
        }
        break;
    }
}

void KristalliECModule::HandleLoginReplyMessage(MsgLoginReply& msg)
{
    if (msg.success != 0)
    {
        LogInfo("Login to EC server successful, connection ID is " + ToString<uint>(msg.userID));
        loginOk_ = true;
        
        // Login successful, begin to handle EC replication
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene)
            RegisterToComponentChangeSignals(scene);
    }
    else
    {
        LogInfo("Login to EC server was denied");
        loginOk_ = false;
    }
}

void KristalliECModule::HandleComponentUpdatedMessage(MsgComponentUpdated& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    RexUUID fullID;
    memcpy(fullID.data, msg.entityUUID, 16);

    Scene::EntityPtr entity = LookUpEntity(msg.entityID, fullID);
    
    // If entity did not exist before, and it's not a prim, can create it now.
    // Otherwise have to wait for the prim entity to be created via opensim
    if (!entity)
    {
        if (fullID.IsNull())
        {
            if (!entity)
            {
                entity = scene->CreateEntity(msg.entityID);
                if (!entity)
                {
                    LogError("Could not create entity");
                    return;
                }
            }
        }
        else
        {
            if (pendingComponentUpdates_.size() > MAX_PENDING_EC_UPDATES)
                pendingComponentUpdates_.erase(pendingComponentUpdates_.begin());
            pendingComponentUpdates_.push_back(msg);
            return;
        }
    }

    std::string componentTypeName = BufferToString(msg.componentTypeName);
    std::string componentName = BufferToString(msg.componentName);
    
    // Get or create the component
    Foundation::ComponentPtr comp = entity->GetComponent(componentTypeName, componentName);
    if (!comp)
    {
        Foundation::ComponentManagerPtr compMgr = framework_->GetComponentManager();
        comp = compMgr->CreateComponent(componentTypeName);
        if (!comp)
        {
            LogError("Could not create component " + componentTypeName);
            return;
        }
        comp->SetName(componentName);
        entity->AddComponent(comp);
    }
    if (!comp->IsSerializable())
    {
        LogError("Received component data for non-serializable component " + componentTypeName);
        return;
    }
    
    QDomDocument temp_doc;
    if (temp_doc.setContent(QByteArray::fromRawData((const char*)&msg.componentData[0], msg.componentData.size())))
    {
        // There should be one entity element & one component element, since components are serialized one by one
        QDomElement entity_elem = temp_doc.firstChildElement("entity");
        if (entity_elem.isNull())
        {
            LogError("No entity element in serialized EC xml data");
            return;
        }
        QDomElement comp_elem = entity_elem.firstChildElement("component");
        if (comp_elem.isNull())
        {
            LogError("No component element in serialized EC xml data");
            return;
        }
        comp->DeserializeFrom(comp_elem, AttributeChange::Network);
        comp->ComponentChanged(AttributeChange::Network);
    }
    else
        LogError("Could not parse EC xml data");
}

void KristalliECModule::HandleComponentDeletedMessage(MsgComponentDeleted& msg)
{
    RexUUID fullID;
    memcpy(fullID.data, msg.entityUUID, 16);
    
    Scene::EntityPtr entity = LookUpEntity(msg.entityID, fullID);
    if (!entity)
        return;
        
    std::string componentTypeName = BufferToString(msg.componentTypeName);
    std::string componentName = BufferToString(msg.componentName);
    
    Foundation::ComponentPtr comp = entity->GetComponent(componentTypeName, componentName);
    if (!comp)
        return;
    entity->RemoveComponent(comp);
}

void KristalliECModule::HandleEntityUpdatedMessage(MsgEntityUpdated& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    RexUUID fullID;
    memcpy(fullID.data, msg.entityUUID, 16);
    
    // If this entity does not exist yet, and it is not a prim, create it
    Scene::EntityPtr entity = LookUpEntity(msg.entityID, fullID);
    if ((!entity) && (fullID.IsNull()))
        entity = scene->CreateEntity(msg.entityID);
    //! \todo permissions update
}

void KristalliECModule::HandleEntityDeletedMessage(MsgEntityDeleted& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    RexUUID fullID;
    memcpy(fullID.data, msg.entityUUID, 16);
    // The Kristalliserver should not be, for now, authoritative of a prim being removed from the world, so check & disregard that case
    Scene::EntityPtr entity = LookUpEntity(msg.entityID, fullID);
    if (entity)
    {
        bool isPrim = (entity->GetComponent("EC_OpenSimPrim").get() != 0);
        if (!isPrim)
            scene->RemoveEntity(msg.entityID);
        else
        {
            // However, we should delete all serializable components from the entity, because that's what the Kristalliserver *is* authoritative over
            std::vector<Foundation::ComponentPtr> comps = entity->GetComponentVector();
            for (uint i = 0; i < comps.size(); ++i)
            {
                if (comps[i]->IsSerializable())
                    entity->RemoveComponent(comps[i]);
            }
        }
    }
}

void KristalliECModule::HandleSceneEvent(event_id_t id, Foundation::EventDataInterface* data)
{
    MessageConnection* connection = GetConnection();
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if ((!loginOk_) || (!connection) || (!scene))
        return;
        
    // This does not at least currently map to any scenemanager signal nicely, so have to use a scene event
    //switch (id)
    //{
    //case Scene::Events::EVENT_ENTITY_CREATE_WITH_ECS:
    //    {
    //        Scene::Events::CreateEntityWithECsEventData* event_data = checked_static_cast<Scene::Events::CreateEntityWithECsEventData*>(data);
    //        MsgCreateEntity msg;
    //        msg.actionID = GetNextActionID();
    //        msg.initialComponents.resize(event_data->components.size());
    //        for (unsigned i = 0; i < event_data->components.size(); ++i)
    //        {
    //            msg.initialComponents[i].componentTypeName = StringToBuffer(event_data->components[i].componentTypeName);
    //            msg.initialComponents[i].componentName = StringToBuffer(event_data->components[i].componentName);
    //            msg.initialComponents[i].componentData = event_data->components[i].componentData;
    //        }
    //        
    //        connection->Send(msg);
    //    }
    //    break;
    //}
}

u32 KristalliECModule::GetNextActionID()
{
    actionID_++;
    return actionID_;
}

Scene::EntityPtr KristalliECModule::LookUpEntity(entity_id_t id, const RexUUID& fullid)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return Scene::EntityPtr();
    
    // If fullid is null, do lookup by local ID only
    if (fullid.IsNull())
    {
        return scene->GetEntity(id);
    }
    
    // Else search for a prim with specified full ID
    // Note: this means that our local ID may mismatch with Kristalliserver's local ID, but it does not matter
    boost::shared_ptr<Foundation::WorldLogicInterface> worldlogic = 
        framework_->GetServiceManager()->GetService<Foundation::WorldLogicInterface>(Foundation::Service::ST_WorldLogic).lock();
    if (worldlogic)
        return worldlogic->GetPrimEntity(fullid);
    else
        return Scene::EntityPtr();
}

RexUUID KristalliECModule::GetEntityFullID(Scene::Entity* entity)
{
    if (!entity)
        return RexUUID();
        
    EC_OpenSimPrim* prim = entity->GetComponent<EC_OpenSimPrim>().get();
    if (prim)
    {
        return prim->FullId;
    }
    else
        return RexUUID();
}

void KristalliECModule::HandlePendingComponentUpdates()
{
    uint idx = 0;
    while (idx < pendingComponentUpdates_.size())
    {
        bool done = false;
        MsgComponentUpdated& msg = pendingComponentUpdates_[idx];
        RexUUID fullID;
        memcpy(fullID.data, msg.entityUUID, 16);
        Scene::EntityPtr entity = LookUpEntity(msg.entityID, fullID);
        if (entity)
        {
            // If we can find the prim entity now, the message should be safe to apply
            HandleComponentUpdatedMessage(msg);
            done = true;
        }
        
        if (done)
            pendingComponentUpdates_.erase(pendingComponentUpdates_.begin() + idx);
        else
            idx++;
    }
}

} // ~KristalliEC namespace

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace KristalliEC;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(KristalliECModule)
POCO_END_MANIFEST
