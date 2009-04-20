// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <sstream>

#include "NetworkEventHandler.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"
//#include "SceneModule.h"
#include "SceneEvents.h"
#include "Entity.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OpenSimAvatar.h"

// Ogre renderer -specific.
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"

#include "QuatUtils.h"
#include "ConversionUtils.h"
#include "BitStream.h"
#include "Terrain.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>

namespace
{
    /// Clones a new Ogre material that renders using the given ambient color. This function will be removed or refactored later on, once proper material system is present. -jj.
    void DebugCreateAmbientColorMaterial(const std::string &materialName, float r, float g, float b)
    {
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);
        if (material.get())
            return;

        material = mm.getByName("SolidAmbient");
        if (!material.get())
            return;

        Ogre::MaterialPtr newMaterial = material->clone(materialName);
        newMaterial->setAmbient(r, g, b);
    }
}

namespace RexLogic
{
    NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;

        ///\todo weak_pointerize
        netInterface_ = dynamic_cast<OpenSimProtocol::OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol).lock().get());
        if (!netInterface_)
        {
            RexLogicModule::LogError("NetworkEventHandler: Could not acquire OpenSimProtocolModule!.");
            return;
        }

        DebugCreateAmbientColorMaterial("AmbientWhite", 1.f, 1.f, 1.f);
        DebugCreateAmbientColorMaterial("AmbientGreen", 0.f, 1.f, 0.f);
        DebugCreateAmbientColorMaterial("AmbientRed", 1.f, 0.f, 0.f);
    }

    NetworkEventHandler::~NetworkEventHandler()
    {

    }

    /// Creates a bounding box (consisting of lines) into the Ogre scene hierarchy. This function will be removed or refactored later on, once proper material system is present. -jj.
    void NetworkEventHandler::DebugCreateOgreBoundingBox(Foundation::ComponentInterfacePtr ogrePlaceable, const std::string &materialName)
    {
        OgreRenderer::EC_OgrePlaceable &component = dynamic_cast<OgreRenderer::EC_OgrePlaceable&>(*ogrePlaceable.get());
        OgreRenderer::Renderer *renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        ///\todo Quick W.I.P Ogre object naming, refactor. -jj.
        static int c = 0;
        std::stringstream ss;
        ss << "manual " << c++;
        Ogre::ManualObject *manual = sceneMgr->createManualObject(ss.str());
        manual->begin(materialName, Ogre::RenderOperation::OT_LINE_LIST);

        const Ogre::Vector3 v[8] = 
        {
            Ogre::Vector3(-1,-1,-1), // 0 ---
            Ogre::Vector3(-1,-1, 1), // 1 --+
            Ogre::Vector3(-1, 1,-1), // 2 -+-
            Ogre::Vector3(-1, 1, 1), // 3 -++
            Ogre::Vector3( 1,-1,-1), // 4 +--
            Ogre::Vector3( 1,-1, 1), // 5 +-+
            Ogre::Vector3( 1, 1,-1), // 6 ++-
            Ogre::Vector3( 1, 1, 1), // 7 +++
        };

        manual->position(v[0]);
        manual->position(v[1]);
        manual->position(v[0]);
        manual->position(v[2]);
        manual->position(v[0]);
        manual->position(v[4]);

        manual->position(v[1]);
        manual->position(v[3]);
        manual->position(v[1]);
        manual->position(v[5]);

        manual->position(v[2]);
        manual->position(v[6]);
        manual->position(v[2]);
        manual->position(v[3]);

        manual->position(v[3]);
        manual->position(v[7]);

        manual->position(v[4]);
        manual->position(v[5]);
        manual->position(v[4]);
        manual->position(v[6]);

        manual->position(v[5]);
        manual->position(v[7]);

        manual->position(v[6]);
        manual->position(v[7]);

        manual->end();
        manual->setDebugDisplayEnabled(true);
       
        Ogre::SceneNode *node = component.GetSceneNode();
        node->attachObject(manual);
    }

    void NetworkEventHandler::DebugCreateTerrainVisData(const DecodedTerrainPatch &patch, int patchSize)
    {
        if (patch.heightData.size() < patchSize * patchSize)
        {
            RexLogicModule::LogWarning("Not enough height map data to fill patch points!");
            return;
        }

        OgreRenderer::Renderer *renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        ///\todo Quick W.I.P Ogre object naming, refactor. -jj.
        static int c = 0;
        std::stringstream ss;
        ss.clear();
        ss << "terrain " << c++;
        Ogre::ManualObject *manual = sceneMgr->createManualObject(ss.str());
        manual->begin("AmbientWhite", Ogre::RenderOperation::OT_LINE_LIST);

        const float vertexSpacingX = 1.f;
        const float vertexSpacingY = 1.f;
        const float patchSpacingX = 16 * vertexSpacingX;
        const float patchSpacingY = 16 * vertexSpacingY;
        const Ogre::Vector3 patchOrigin(patch.header.x * patchSpacingX, 0.f, patch.header.y * patchSpacingY);
        const float heightScale = 1.f;
        for(int y = 0; y+1 < patchSize; ++y)
            for(int x = 0; x+1 < patchSize; ++x)
            {
                Ogre::Vector3 a = patchOrigin + Ogre::Vector3(vertexSpacingX * x,     heightScale * patch.heightData[y*patchSize+x],   vertexSpacingY * y);
                Ogre::Vector3 b = patchOrigin + Ogre::Vector3(vertexSpacingX * (x+1), heightScale * patch.heightData[y*patchSize+x+1], vertexSpacingY * y);
                Ogre::Vector3 c = patchOrigin + Ogre::Vector3(vertexSpacingX * x,     heightScale * patch.heightData[(y+1)*patchSize+x], vertexSpacingY * (y+1));
                Ogre::Vector3 d = patchOrigin + Ogre::Vector3(vertexSpacingX * (x+1), heightScale * patch.heightData[(y+1)*patchSize+x+1], vertexSpacingY * (y+1));

                manual->position(a);
                manual->position(b);
                manual->position(a);
                manual->position(c);
//                manual->position(a);
//                manual->position(d);

                if (x+2 == patchSize)
                {
                    manual->position(b);
                    manual->position(d);
                }
                if (y+2 == patchSize)
                {
                    manual->position(c);
                    manual->position(d);
                }
            }

        manual->end();
        manual->setDebugDisplayEnabled(true);
       
        Ogre::SceneNode *node = sceneMgr->createSceneNode();
        sceneMgr->getRootSceneNode()->addChild(node);
        node->attachObject(manual);
    }

    bool NetworkEventHandler::HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        OpenSimProtocol::NetworkEventInboundData *netdata = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
        switch(event_id)
        {
            case RexNetMsgRegionHandshake:              return HandleOSNE_RegionHandshake(netdata); break;
            case RexNetMsgAgentMovementComplete:        return HandleOSNE_AgentMovementComplete(netdata); break;
            case RexNetMsgGenericMessage:               return HandleOSNE_GenericMessage(netdata); break;
            case RexNetMsgLogoutReply:                  return HandleOSNE_LogoutReply(netdata); break;
            case RexNetMsgImprovedTerseObjectUpdate:    return HandleOSNE_ImprovedTerseObjectUpdate(netdata); break;                
            case RexNetMsgObjectUpdate:                 return HandleOSNE_ObjectUpdate(netdata); break;
            case RexNetMsgObjectProperties:             return HandleOSNE_ObjectProperties(netdata); break;
            case RexNetMsgLayerData:                    return HandleOSNE_LayerData(netdata); break;
            default:                                    return false; break;
        }
        
        return false;
    }

    Foundation::EntityPtr NetworkEventHandler::GetPrimEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        if (!scene)
            return Foundation::EntityPtr();

        Foundation::EntityPtr entity = scene->GetEntity(entityid);

        ///\todo Check that the entity has a prim component, if not, add it to the entity. 
        return entity;
    }
  
    Foundation::EntityPtr NetworkEventHandler::GetOrCreatePrimEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        Core::event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
        
        Foundation::EntityPtr entity = scene->GetEntity(entityid);
        if (!entity)
        {
            // Create a new entity.
            Foundation::EntityPtr entity = CreateNewPrimEntity(entityid);
            UUIDs_[fullid] = entityid;
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent("EC_OpenSimPrim").get());
            prim.LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
            prim.FullId = fullid;
            return entity;
        }

        ///\todo Check that the entity has a prim component, if not, add it to the entity.

        // Send the 'Entity Updated' event.
        /*Foundation::ComponentInterfacePtr component = entity->GetComponent("EC_OpenSimPrim");
        EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(component.get());
        Scene::SceneEventData::Events entity_event_data(entityid);
        entity_event_data.sceneName = scene->Name();
        framework_->GetEventManager()->SendEvent(cat_id, Scene::Events::EVENT_ENTITY_UPDATED, &entity_event_data);*/
        
        return entity;
    }  
   
    Foundation::EntityPtr NetworkEventHandler::GetPrimEntity(const RexUUID &entityuuid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        IDMap::iterator iter = UUIDs_.find(entityuuid);
        if (iter == UUIDs_.end())
            return Foundation::EntityPtr();
        else
            return scene->GetEntity(iter->second);
    }    
    
    
    Foundation::EntityPtr NetworkEventHandler::CreateNewPrimEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimPrim::NameStatic());
        defaultcomponents.push_back(EC_Viewable::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents); 

        DebugCreateOgreBoundingBox(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()),"AmbientRed");
        return entity;
    }
    
    Foundation::EntityPtr NetworkEventHandler::GetOrCreateAvatarEntity(Core::entity_id_t entityid, const RexUUID &fullid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        Foundation::EntityPtr entity = scene->GetEntity(entityid);
        if (!entity)
        {
            entity = CreateNewAvatarEntity(entityid);
            UUIDs_[fullid] = entityid;
            EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent("EC_OpenSimAvatar").get());
            avatar.LocalId = entityid; ///\note In current design it holds that localid == entityid, but I'm not sure if this will always be so?
            avatar.FullId = fullid;
        }
        return entity;
    }    

    Foundation::EntityPtr NetworkEventHandler::CreateNewAvatarEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");
        
        Core::StringVector defaultcomponents;
        defaultcomponents.push_back(EC_OpenSimAvatar::NameStatic());
        defaultcomponents.push_back(OgreRenderer::EC_OgrePlaceable::NameStatic());        
        
        Foundation::EntityPtr entity = scene->CreateEntity(entityid,defaultcomponents);
 
        OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());
        ogrePos.SetScale(Vector3(0.5,1.5,0.5));
        DebugCreateOgreBoundingBox(entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()),"AmbientGreen");
 
        return entity;
    }

    Foundation::EntityPtr NetworkEventHandler::GetAvatarEntity(Core::entity_id_t entityid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        if (!scene)
            return Foundation::EntityPtr();

        Foundation::EntityPtr entity = scene->GetEntity(entityid);

        ///\todo Check that the entity has a avatar component, if not, add it to the entity. 
        return entity;
    }

    Foundation::EntityPtr NetworkEventHandler::GetAvatarEntity(const RexUUID &entityuuid)
    {
        Foundation::SceneManagerServiceInterface *sceneManager = framework_->GetService<Foundation::SceneManagerServiceInterface>
            (Foundation::Service::ST_SceneManager);
        Foundation::ScenePtr scene = sceneManager->GetScene("World");

        IDMap::iterator iter = UUIDs_.find(entityuuid);
        if (iter == UUIDs_.end())
            return Foundation::EntityPtr();
        else
            return scene->GetEntity(iter->second);
    } 

    bool NetworkEventHandler::HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
 
        msg->ResetReading();
        uint64_t regionhandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.

        uint32_t localid = msg->ReadU32(); 
        msg->SkipToNextVariable();		// State U8 ///\todo Unhandled inbound variable 'State'.
        RexUUID fullid = msg->ReadUUID();
        msg->SkipToNextVariable();		// CRC U32 ///\todo Unhandled inbound variable 'CRC'.
        uint8_t pcode = msg->ReadU8();
        
        Foundation::EntityPtr entity;
        switch(pcode)
        {
            // Prim
            case 0x09:
            {
                entity = GetOrCreatePrimEntity(localid,fullid);
                EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent("EC_OpenSimPrim").get());
                OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());

                ///\todo Are we setting the param or looking up by this param? I think the latter, but this is now doing the former. 
                ///      Will cause problems with multigrid support.
                prim.RegionHandle = regionhandle; 

                prim.Material = msg->ReadU8();
                prim.ClickAction = msg->ReadU8();
                ogrePos.SetScale(Core::OpenSimToOgreCoordinateAxes(msg->ReadVector3()));
                
                size_t bytes_read = 0;
                const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
                if (bytes_read == 60)
                {
                    // The data contents:
                    // ofs  0 - pos xyz - 3 x float (3x4 bytes)
                    // ofs 12 - vel xyz - 3 x float (3x4 bytes)
                    // ofs 24 - acc xyz - 3 x float (3x4 bytes)
                    // ofs 36 - orientation, quat with last (w) component omitted - 3 x float (3x4 bytes)
                    // ofs 48 - angular velocity - 3 x float (3x4 bytes)
                    // total 60 bytes
                    Core::Vector3df pos = *reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[0]);
                    ogrePos.SetPosition(Core::OpenSimToOgreCoordinateAxes(pos));
                    Core::Quaternion quat = UnpackQuaternionFromFloat3((float*)&objectdatabytes[36]); 
                    ogrePos.SetOrientation(Core::OpenSimToOgreQuaternion(quat));

                    /// \todo Velocity field unhandled.
                    /// \todo Acceleration field unhandled.
                    /// \todo Angular velocity field unhandled.
                }
                else
                    RexLogicModule::LogError("Error reading ObjectData for prim:" + Core::ToString(prim.LocalId) + ". Bytes read:" + Core::ToString(bytes_read));
                
                prim.ParentId = msg->ReadU32();
                prim.UpdateFlags = msg->ReadU32();
                
                // Skip path related variables
                msg->SkipToFirstVariableByName("Text");
                prim.HoveringText = msg->ReadString(); 
                msg->SkipToNextVariable();      // TextColor
                prim.MediaUrl = msg->ReadString();
            }
            break;
            // Avatar                
            case 0x2f:
                entity = GetOrCreateAvatarEntity(localid,fullid);
                EC_OpenSimAvatar &avatar = *checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent("EC_OpenSimAvatar").get());
                OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());

                avatar.RegionHandle = regionhandle;
                
                // Get position from objectdata
                msg->SkipToFirstVariableByName("ObjectData");
                size_t bytes_read = 0;
                const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
                if (bytes_read >= 28)
                {
                    // The data contents:
                    // ofs 16 - pos xyz - 3 x float (3x4 bytes)
                    Core::Vector3df pos = *reinterpret_cast<const Core::Vector3df*>(&objectdatabytes[16]);
                    ogrePos.SetPosition(Core::OpenSimToOgreCoordinateAxes(pos));
                }                
                
                msg->SkipToFirstVariableByName("ParentID");
                avatar.ParentId = msg->ReadU32();
                
                // NameValue contains: FirstName STRING RW SV " + firstName + "\nLastName STRING RW SV " + lastName
                msg->SkipToFirstVariableByName("NameValue");
                std::string namevalue = msg->ReadString();
                size_t pos = namevalue.find("\n");
                if(pos != std::string::npos)
                {
                    avatar.FirstName = namevalue.substr(23,pos-23);
                    avatar.LastName = namevalue.substr(pos+23);
                } 
                break;
        }

        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ObjectProperties(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage *msg = data->message;
        msg->ResetReading();
        
        RexUUID full_id = msg->ReadUUID();
        msg->SkipToFirstVariableByName("Name");
        std::string name = msg->ReadString();
        std::string desc = msg->ReadString();
        ///\todo Handle rest of the vars.
        
        Foundation::EntityPtr entity = GetPrimEntity(full_id);
        if(entity)
        {
            EC_OpenSimPrim &prim = *checked_static_cast<EC_OpenSimPrim*>(entity->GetComponent("EC_OpenSimPrim").get());
            prim.ObjectName = name;
            prim.Description = desc;
            
            // Send the 'Entity Selected' event.
            Core::event_category_id_t event_category_id = framework_->GetEventManager()->QueryEventCategory("Scene");
            Scene::Events::SceneEventData event_data(prim.LocalId);
            framework_->GetEventManager()->SendEvent(event_category_id, Scene::Events::EVENT_ENTITY_SELECTED, &event_data);
        }
        else
            RexLogicModule::LogInfo("Received 'ObjectProperties' packet for unknown entity (" + full_id.ToString() + ").");
        
        return false;        
    }
    
    bool NetworkEventHandler::HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data)
    {        
        data->message->ResetReading();    
        data->message->SkipToNextVariable();      // AgentId
        data->message->SkipToNextVariable();      // SessionId
        data->message->SkipToNextVariable();      // TransactionId
        std::string methodname = data->message->ReadString(); 

        if(methodname == "RexMediaUrl")
            return HandleRexGM_RexMediaUrl(data);
        else if(methodname == "RexPrimData")
            return HandleRexGM_RexPrimData(data); 
        else if(methodname == "RexAppearance")
            return HandleRexGM_RexAppearance(data);
        else
            return false;    
    }

    bool NetworkEventHandler::HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data)
    {
        /// \todo tucofixme
        return false;
    }

    bool NetworkEventHandler::HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        size_t bytes_read;    
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("Parameter");
        RexUUID primuuid(data->message->ReadString());
        
        Foundation::EntityPtr entity = GetPrimEntity(primuuid);
        if(entity)
        {
            // Calculate full data size
            size_t fulldatasize = 0;        
            NetVariableType nextvartype = data->message->CheckNextVariableType();
            while(nextvartype != NetVarNone)
            {
                data->message->ReadBuffer(&bytes_read);
                fulldatasize += bytes_read;
                nextvartype = data->message->CheckNextVariableType();
            }
           
            size_t bytes_read;
            const uint8_t *readbytedata;
            data->message->ResetReading();
            data->message->SkipToFirstVariableByName("Parameter");
            data->message->SkipToNextVariable();            // Prim UUID
            
            uint8_t *fulldata = new uint8_t[fulldatasize];
            int pos = 0;
            nextvartype = data->message->CheckNextVariableType();
            while(nextvartype != NetVarNone)
            {
                readbytedata = data->message->ReadBuffer(&bytes_read);
                memcpy(fulldata+pos,readbytedata,bytes_read);
                pos += bytes_read;
                nextvartype = data->message->CheckNextVariableType();
            }

            Foundation::ComponentInterfacePtr oscomponent = entity->GetComponent("EC_OpenSimPrim");
            checked_static_cast<EC_OpenSimPrim*>(oscomponent.get())->HandleRexPrimData(fulldata);
            /// \todo tucofixme, checked_static_cast<EC_OpenSimPrim*>(oscomponent.get())->PrintDebug();

            Foundation::ComponentInterfacePtr viewcomponent = entity->GetComponent("EC_Viewable");
            checked_static_cast<EC_Viewable*>(viewcomponent.get())->HandleRexPrimData(fulldata);
            /// \todo tucofixme, checked_static_cast<EC_Viewable*>(viewcomponent.get())->PrintDebug();
            
            delete fulldata;
        }
        return false;
    }
    
    /// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
    bool NetworkEventHandler::HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage &msg = *data->message;
        u8 layerID = msg.ReadU8();
        size_t sizeBytes = 0;
        const uint8_t *packedData = msg.ReadBuffer(&sizeBytes);
        BitStream bits(packedData, sizeBytes);
        TerrainPatchGroupHeader header;

        header.stride = bits.ReadBits(16);
        header.patchSize = bits.ReadBits(8);
        header.layerType = bits.ReadBits(8);

        switch(header.layerType)
        {
        case TPLayerLand:
        {
            std::vector<DecodedTerrainPatch> patches;
            DecompressLand(patches, bits, header);
            for(size_t i = 0; i < patches.size(); ++i)
                DebugCreateTerrainVisData(patches[i], header.patchSize);
            break;
        }
        default:
            ///\todo Log out warning - unhandled packet type.
            break;
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data)    
    {
        size_t bytesRead = 0;

        data->message->ResetReading();    
        data->message->SkipToNextVariable(); // RegionFlags U32
        data->message->SkipToNextVariable(); // SimAccess U8

        std::string simname = data->message->ReadString();
        rexlogicmodule_->GetServerConnection()->simname_ = simname;
        
        RexLogicModule::LogInfo("Joined to the sim \"" + simname + "\".");
        
        // Create the "World" scene.

        const ClientParameters& client = netInterface_->GetClientParameters();
        rexlogicmodule_->GetServerConnection()->SendRegionHandshakeReplyPacket(client.agentID, client.sessionID, 0);   
        return false;  
    } 

    bool NetworkEventHandler::HandleOSNE_LogoutReply(OpenSimProtocol::NetworkEventInboundData* data)   
    {
        data->message->ResetReading();
        RexUUID aID = data->message->ReadUUID();
        RexUUID sID = data->message->ReadUUID();

        if (aID == rexlogicmodule_->GetServerConnection()->GetInfo().agentID &&
            sID == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
        {
            RexLogicModule::LogInfo("LogoutReply received with matching IDs. Logging out.");
            rexlogicmodule_->GetServerConnection()->CloseServerConnection();
        } 
        return false;   
    } 
  
    bool NetworkEventHandler::HandleOSNE_AgentMovementComplete(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();

        RexUUID agentid = data->message->ReadUUID();
        RexUUID sessionid = data->message->ReadUUID();
        
        if (agentid == rexlogicmodule_->GetServerConnection()->GetInfo().agentID && sessionid == rexlogicmodule_->GetServerConnection()->GetInfo().sessionID)
        {
            Vector3 position = data->message->ReadVector3(); /// \todo tucofixme, set position to avatar
            Vector3 lookat = data->message->ReadVector3(); /// \todo tucofixme, set lookat direction to avatar
            /// \todo tuco, use OpenSimToOgreCoordinateAxes to convert pos and scale, and OpenSimToOgreQuaternion to convert orientation to our system. 
            uint64_t regionhandle = data->message->ReadU64();
            uint32_t timestamp = data->message->ReadU32(); 
        }
        return false;
    }

    bool NetworkEventHandler::HandleOSNE_ImprovedTerseObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();   
    
        uint64_t regionhandle = data->message->ReadU64();    
        data->message->SkipToNextVariable(); // TimeDilation U16 ///\todo Unhandled inbound variable 'TimeDilation'.
  
        NetVariableType nextvartype = data->message->CheckNextVariableType();
        while(nextvartype != NetVarNone)
        {
            size_t bytes_read = 0;
            const uint8_t *bytes = data->message->ReadBuffer(&bytes_read);

            // 30 is size for rex's own avatarimprovedterseupdate
            if (bytes_read == 30)
            {
                // The data contents:
                // ofs  0 - localid - packed to 4 bytes
                // ofs  4 - position xyz - 3 x float (3x4 bytes)
                // ofs 16 - velocity xyz - packed to 6 bytes
                // ofs 22 - rotation - packed to 8 bytes
                
                //! \todo handle endians
                int i = 0;
                uint32_t localid = *reinterpret_cast<uint32_t*>((uint32_t*)&bytes[i]);                
                i += 4;

                Core::Vector3df position = *reinterpret_cast<Core::Vector3df*>((Core::Vector3df*)&bytes[i]);
                i += sizeof(Core::Vector3df);
                
                //! \todo read velocity & rotation 
                
                Foundation::EntityPtr entity = GetAvatarEntity(localid);
                if(entity)
                {
                    OgreRenderer::EC_OgrePlaceable &ogrePos = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(entity->GetComponent("EC_OgrePlaceable").get());
                    ogrePos.SetPosition(Core::OpenSimToOgreCoordinateAxes(position));
                }
            }
            
            data->message->SkipToNextVariable(); // TextureEntry variable ///\todo Unhandled inbound variable 'TextureEntry'.
            nextvartype = data->message->CheckNextVariableType();
        }
        return false;
    }
    
    bool NetworkEventHandler::HandleRexGM_RexAppearance(OpenSimProtocol::NetworkEventInboundData* data)
    {
        data->message->ResetReading();    
        data->message->SkipToFirstVariableByName("Parameter");
        
        std::string avataraddress = data->message->ReadString();
        RexUUID avatarid(data->message->ReadString());
        bool overrideappearance = Core::ParseString<bool>(data->message->ReadString());
        return false;
    }
        
    
}
