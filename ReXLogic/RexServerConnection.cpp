// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include "RexServerConnection.h"
#include "RexLogicModule.h"
#include "RexProtocolMsgIDs.h"
#include "EC_OgrePlaceable.h"
#include "EC_OpenSimPrim.h"
#include "QuatUtils.h"
#include "ConversionUtils.h"
#include "SceneManager.h"

namespace RexLogic
{
    RexServerConnection::RexServerConnection(Foundation::Framework *framework)
    {
        framework_ = framework;    
        connected_ = false;
        state_ = OpenSimProtocol::Connection::STATE_DISCONNECTED;
        
        serverAddress_ = "";
        serverPort_ = 0;
        
        myInfo_.agentID.SetNull();
        myInfo_.sessionID.SetNull();
        myInfo_.regionID.SetNull();
        myInfo_.circuitCode = 0;

		connection_type_ = DirectConnection;
        
        // Get pointer to the network interface.
        netInterface_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::OpenSimProtocolModule>
            (Foundation::Module::MT_OpenSimProtocol);
        
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return;
        }
    }

    RexServerConnection::~RexServerConnection()
    {
    }
    
	bool RexServerConnection::ConnectToServer(const std::string& username, const std::string& password, 
				const std::string& serveraddress, const std::string& auth_server_address, const std::string& auth_login)
    {
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return false;
        }
                
        if(connected_)
        {
            RexLogicModule::LogError("Already connected.");
            return false;        
        }

        size_t pos = username.find(" ");
        if(pos == std::string::npos)
        {
            RexLogicModule::LogError("Invalid username, last name not found." + username);
            return false;
        }
        
        std::string first_name = username.substr(0, pos);
        std::string last_name = username.substr(pos + 1);
        
        // Get server address and port.
        int port = 9000;
        pos = serveraddress.find(":");
        std::string serveraddress_noport;
        if(pos == std::string::npos)
        {
            serveraddress_noport = serveraddress;
            RexLogicModule::LogInfo("No port defined for the server, using default port (9000).");
        }
        else
        {
            serveraddress_noport = serveraddress.substr(0, pos);
        
            try
            {
                port = boost::lexical_cast<int>(serveraddress.substr(pos + 1));
            }catch(std::exception)
            {
                RexLogicModule::LogError("Invalid port number, only numbers are allowed.");
                return false;
            }
        }
		
		bool connect_result = false;
		if (auth_server_address != "") 
		{
			connection_type_ = AuthenticationConnection;
			connect_result = sp->LoginUsingRexAuthentication(first_name,
				last_name, password, serveraddress_noport, port,
				auth_server_address, auth_login, &threadState_);
		}

		if (connection_type_ == DirectConnection)
            sp->LoginToServer(first_name, last_name,password, serveraddress_noport, port, &threadState_);
        
        // Save the server address and port for later use.
        serverAddress_ = serveraddress_noport;
        serverPort_ = port;
        
        return true;
    }
    
    bool RexServerConnection::CreateUDPConnection()
    {
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return false;
        }
        
        bool connect_result = sp->CreateUDPConnection(serverAddress_.c_str(), serverPort_);

		if(connect_result)
		{
			connected_ = true;
            
            // Get the client-spesific information.
			myInfo_ = sp->GetClientParameters();
			
			// Check that the parameters are valid.
			if (myInfo_.agentID.IsNull() || myInfo_.sessionID.IsNull())
			{
			    // Client parameters not valid. Disconnect.
			    sp->DisconnectFromRexServer();
			    RexLogicModule::LogError("Client parameters are not valid! Disconnecting.");
			    connected_ = false;
			    return false;
            }
			
			// Send the necessary UDP packets.
			SendUseCircuitCodePacket();
			SendCompleteAgentMovementPacket();
			
			RexLogicModule::LogInfo("Connected to server " + serverAddress_ + ".");
		}
		else
		{
			RexLogicModule::LogInfo("Connecting to server " + serverAddress_ + " failed.");
        }
        
        return connect_result;
    }
    
    void RexServerConnection::RequestLogout()
    {
        if(!connected_)
            return;    

        SendLogoutRequestPacket();
    } 

    void RexServerConnection::ForceServerDisconnect()
    {
        if(!connected_)
            return;
            
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        sp->DisconnectFromRexServer(); 
        
        connected_ = false;
    }
    
	void RexServerConnection::SendUseCircuitCodePacket()
	{
        if(!connected_)
            return;	

        NetOutMessage *m = StartMessageBuilding(RexNetMsgUseCircuitCode);
        assert(m);
        
        m->AddU32(myInfo_.circuitCode);
        m->AddUUID(myInfo_.sessionID);
        m->AddUUID(myInfo_.agentID);
        
        FinishMessageBuilding(m);
	}

    void RexServerConnection::SendCompleteAgentMovementPacket()
    {
        if(!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgCompleteAgentMovement);
        assert(m);
        
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddU32(myInfo_.circuitCode);
        
        FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendLogoutRequestPacket()
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgLogoutRequest);
        assert(m);
        
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);

        FinishMessageBuilding(m);
	}
	
	void RexServerConnection::SendChatFromViewerPacket(std::string text)
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgChatFromViewer);
        assert(m);
        
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddBuffer(text.length(), (uint8_t*)text.c_str());
        m->AddU8(1);
        m->AddS32(0);
        
        FinishMessageBuilding(m);
	}	
	
	void RexServerConnection::SendAgentUpdatePacket(Core::Quaternion bodyrot, Core::Quaternion headrot, uint8_t state, 
	    RexTypes::Vector3 camcenter, RexTypes::Vector3 camataxis, RexTypes::Vector3 camleftaxis, RexTypes::Vector3 camupaxis,
	    float fardist, uint32_t controlflags, uint8_t flags)
    {
        if(!connected_)
            return;    
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgAgentUpdate);
        assert(m);
        
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        m->AddQuaternion(Core::OgreToOpenSimQuaternion(bodyrot));
        m->AddQuaternion(Core::OgreToOpenSimQuaternion(headrot));        
        m->AddU8(state);
        m->AddVector3(camcenter);
        m->AddVector3(camataxis);        
        m->AddVector3(camleftaxis);
        m->AddVector3(camupaxis);
        m->AddF32(fardist);
        m->AddU32(controlflags);
        m->AddU8(flags);
        
        FinishMessageBuilding(m);
	}
    
    void RexServerConnection::SendObjectSelectPacket(Core::entity_id_t object_id)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(object_id);
        
        FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendObjectSelectPacket(std::vector<Core::entity_id_t> object_id_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectSelect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);
        
        FinishMessageBuilding(m);
    }

    void RexServerConnection::SendObjectDeselectPacket(Core::entity_id_t object_id)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(1);
        m->AddU32(object_id);
                        
        FinishMessageBuilding(m);
    }    
    
    void RexServerConnection::SendObjectDeselectPacket(std::vector<Core::entity_id_t> object_id_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDeselect);
        assert(m);
        
        // AgentData
        m->AddUUID(myInfo_.agentID);
        m->AddUUID(myInfo_.sessionID);
        
        // ObjectData
        m->SetVariableBlockCount(object_id_list.size());
        for(size_t i = 0; i < object_id_list.size(); ++i)
            m->AddU32(object_id_list[i]);
                        
        FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendMultipleObjectUpdatePacket(std::vector<Foundation::EntityPtr> entity_ptr_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgMultipleObjectUpdate);
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
            const Foundation::ComponentInterfacePtr &prim_component = entity_ptr_list[i]->GetComponent("EC_OpenSimPrim");
            RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
            const Foundation::ComponentInterfacePtr &ogre_component = entity_ptr_list[i]->GetComponent("EC_OgrePlaceable");
            OgreRenderer::EC_OgrePlaceable *ogre_pos = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
        
            m->AddU32(prim->LocalId);
            m->AddU8(13);

            // Position
		    memcpy(&data[offset], &Core::OgreToOpenSimCoordinateAxes(ogre_pos->GetPosition()), sizeof(Vector3));
		    offset += sizeof(Vector3);
		                
            // Scale
		    memcpy(&data[offset], &Core::OgreToOpenSimCoordinateAxes(ogre_pos->GetScale()), sizeof(Vector3));
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
        
        FinishMessageBuilding(m);
    }
    
    void RexServerConnection::SendObjectNamePacket(std::vector<Foundation::EntityPtr> entity_ptr_list)
    {
        if (!connected_)
            return;
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectName);
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
    
        NetOutMessage *m = StartMessageBuilding(RexNetMsgObjectDescription);
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

    void RexServerConnection::SendRegionHandshakeReplyPacket(RexTypes::RexUUID agent_id, RexTypes::RexUUID session_id, uint32_t flags)
    {
        if (!connected_)
            return;    

        NetOutMessage *m = StartMessageBuilding(RexNetMsgRegionHandshakeReply);
        
        m->AddUUID(agent_id);
        m->AddUUID(session_id); 
        m->AddU32(flags);
        
        FinishMessageBuilding(m);
    }
    
    volatile OpenSimProtocol::Connection::State RexServerConnection::GetConnectionState()
    {
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return OpenSimProtocol::Connection::STATE_ENUM_COUNT;
        }
        
        return sp->GetConnectionState();
    }
    
    NetOutMessage *RexServerConnection::StartMessageBuilding(NetMsgID message_id)
    {
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return 0;
        }
        
        return sp->StartMessageBuilding(message_id);
    }
    
    void RexServerConnection::FinishMessageBuilding(NetOutMessage *msg)
    {
        boost::shared_ptr<OpenSimProtocol::OpenSimProtocolModule> sp = netInterface_.lock();
        if (!sp.get())
        {
            RexLogicModule::LogError("Getting network interface did not succeed.");
            return;
        }
        
        sp->FinishMessageBuilding(msg);
    }
}
