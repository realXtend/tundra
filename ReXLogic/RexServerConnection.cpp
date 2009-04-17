// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexServerConnection.h"
#include "RexLogicModule.h"
#include "RexProtocolMsgIDs.h"
#include "EC_OgrePlaceable.h"
#include "EC_OpenSimPrim.h"
#include "QuatUtils.h"
#include "SceneManager.h"

namespace RexLogic
{
    RexServerConnection::RexServerConnection(Foundation::Framework *framework)
    {
        framework_ = framework;    
        connected_ = false;
        
        myInfo_.agentID.SetNull();
        myInfo_.sessionID.SetNull();
        myInfo_.regionID.SetNull();
        myInfo_.circuitCode = 0;

        ///\todo weak_pointerize
        netInterface_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::OpenSimProtocolModule>(Foundation::Module::MT_OpenSimProtocol).lock().get();
        if (!netInterface_)
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return;
        }
    }

    RexServerConnection::~RexServerConnection()
    {
    }
        
    bool RexServerConnection::ConnectToServer(std::string username, std::string password, std::string serverurl)
    {
        if(connected_)
        {
            RexLogicModule::LogError("Already connected.");
            return false;        
        }

        size_t pos = username.find(" ");
        if(pos == std::string::npos)
        {
            RexLogicModule::LogError("Invalid username, last name not found" + username);
            return false;
        }
        
        std::string first_name = username.substr(0, pos);
        std::string last_name = username.substr(pos + 1);
        
        // Get server address and port.
        int port = 9000;
        pos = serverurl.find(":");
        std::string serveraddress_noport;
        if(pos == std::string::npos)
        {
            serveraddress_noport = serverurl;
            RexLogicModule::LogInfo("No port defined in serverurl, using port 9000.");
        }
        else
        {
            serveraddress_noport = serverurl.substr(0, pos);
        
            try
            {
                port = boost::lexical_cast<int>(serverurl.substr(pos + 1));
            }catch(std::exception)
            {
                RexLogicModule::LogError("Invalid port number, only numbers are allowed.");
                return false;
            }
        }

        bool connectresult = netInterface_->ConnectToRexServer(first_name.c_str(), last_name.c_str(),password.c_str(), serveraddress_noport.c_str(), port);   
        if(connectresult)
        {
            connected_ = true;
        
            myInfo_ = netInterface_->GetClientParameters();
            SendUseCircuitCodePacket();
            SendCompleteAgentMovementPacket();
            RexLogicModule::LogInfo("Connected to server " + serveraddress_noport + ".");
            return true;
        }
        else
        {
            RexLogicModule::LogInfo("Connecting to server " + serveraddress_noport + " failed.");
            return false;
        }
    }    
    
    void RexServerConnection::RequestLogout()
    {
        if(!connected_)
            return;    

        SendLogoutRequestPacket();
    } 
 
    void RexServerConnection::CloseServerConnection()
    {
        if(!connected_)
            return;
    
        netInterface_->DisconnectFromRexServer();    
        connected_ = false;
        
        Scene::SceneManager *scene_manager = dynamic_cast<Scene::SceneManager *>
            (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
        
        if (!scene_manager)
            return;
        
        // Delete the scene
        if (scene_manager->HasScene("World"))
        {
            scene_manager->DeleteScene("World");
            assert(!scene_manager->HasScene("World"));
            RexLogicModule::LogInfo("Logged out from server. Scene deleted.");
        }  
    }
    
	void RexServerConnection::SendUseCircuitCodePacket()
	{
        if(!connected_)
            return;	
	
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgUseCircuitCode);
        assert(m);
        m->AddU32(myInfo_.circuitCode);
        m->AddUUID(myInfo_.sessionID);
        m->AddUUID(myInfo_.agentID);
        netInterface_->FinishMessageBuilding(m);
	}

    void RexServerConnection::SendCompleteAgentMovementPacket()
    {
        if(!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgCompleteAgentMovement);
        assert(m);
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddU32(myInfo_.circuitCode);
        netInterface_->FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendLogoutRequestPacket()
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgLogoutRequest);
        assert(m);
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        netInterface_->FinishMessageBuilding(m);
	}
	
	void RexServerConnection::SendChatFromViewerPacket(std::string text)
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgChatFromViewer);
        assert(m);
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddBuffer(text.length(), (uint8_t*)text.c_str());
        m->AddU8(1);
        m->AddS32(0);
        netInterface_->FinishMessageBuilding(m);
	}	
	
	void RexServerConnection::SendAgentUpdatePacket(Core::Quaternion bodyrot, Core::Quaternion headrot, uint8_t state, 
	    RexTypes::Vector3 camcenter, RexTypes::Vector3 camataxis, RexTypes::Vector3 camleftaxis, RexTypes::Vector3 camupaxis,
	    float fardist, uint32_t controlflags, uint8_t flags)
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgAgentUpdate);
        assert(m);
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddQuaternion(bodyrot);
        m->AddQuaternion(headrot);        
        m->AddU8(state);
        m->AddVector3(camcenter);
        m->AddVector3(camataxis);        
        m->AddVector3(camleftaxis);
        m->AddVector3(camupaxis);
        m->AddF32(fardist);
        m->AddU32(controlflags);
        m->AddU8(flags);
        netInterface_->FinishMessageBuilding(m);
	}
    
    void RexServerConnection::SendObjectSelectPacket(Core::entity_id_t object_id)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(object_id);
        
        netInterface_->FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendObjectSelectPacket(std::vector<Core::entity_id_t> object_id_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);
        
        netInterface_->FinishMessageBuilding(m);
    }

    void RexServerConnection::SendObjectDeselectPacket(Core::entity_id_t object_id)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(object_id);
                        
        netInterface_->FinishMessageBuilding(m);
    }    
    
    void RexServerConnection::SendObjectDeselectPacket(std::vector<Core::entity_id_t> object_id_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);
                        
        netInterface_->FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendMultipleObjectUpdatePacket(std::vector<Foundation::EntityPtr> entity_ptr_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);

        // ObjectData        
        ///\todo Update just the necessary parameters (use update flags) & test with multiple objects.
        size_t offset = 0;
        uint8_t data[2048]; ///\todo What is the max size?

        m->SetVariableBlockCount(entity_ptr_list.size());

        for(size_t i = 0; i < entity_ptr_list.size(); ++i)
        {
            // Scale
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
            const Foundation::ComponentInterfacePtr &ogre_component = entity_ptr_list[i]->GetComponent("EC_OgrePlaceable");
            OgreRenderer::EC_OgrePlaceable *ogre_pos = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
        
            m->AddU32(prim->LocalId);
            m->AddU8(13);

            // Position
		    memcpy(&data[offset], &ogre_pos->GetPosition(), sizeof(Vector3));
		    offset += sizeof(Vector3);
		                
            // Scale
		    memcpy(&data[offset], &ogre_pos->GetScale(), sizeof(Vector3));
            offset += sizeof(Vector3);
        }
        
        // Add the data.
        m->AddBuffer(offset, data);        
        
        ///\todo Make this work!
        /*for(size_t i = 0; i < entity_ptr_list.size(); ++i)
        {
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
            const Foundation::ComponentInterfacePtr &ogre_component = entity_ptr_list[i]->GetComponent("EC_OgrePlaceable");
            OgreRenderer::EC_OgrePlaceable *ogre_pos = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
        
            m->AddU32(prim->LocalId);
            m->AddU8(2);
            
            // Rotation
            Vector3 val = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
            memcpy(&data[offset], &val, sizeof(Vector3));            
            offset += sizeof(Vector3);
        }
        
        // Add the data.
        m->AddBuffer(offset, data);*/
        
        netInterface_->FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendObjectNamePacket(std::vector<Foundation::EntityPtr> entity_ptr_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectName);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(entity_ptr_list.size());
        for(size_t i = 0; i < entity_ptr_list.size(); ++i)
        {
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
            m->AddU32(prim->LocalId);
            m->AddBuffer(prim->ObjectName.size(), (uint8_t*)prim->ObjectName.c_str());
        }
    }
    
    void RexServerConnection::SendObjectDescriptionPacket(std::vector<Foundation::EntityPtr> entity_ptr_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgObjectDescription);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(entity_ptr_list.size());
        for(size_t i = 0; i < entity_ptr_list.size(); ++i)
        {
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
            m->AddU32(prim->LocalId);
            m->AddBuffer(prim->Description.size(), (uint8_t*)prim->Description.c_str());
        }
    }        
}
