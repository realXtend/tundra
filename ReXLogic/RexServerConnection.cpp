// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexServerConnection.h"
#include "RexLogicModule.h"
#include "RexProtocolMsgIDs.h"

namespace RexLogic
{
    RexServerConnection::RexServerConnection(Foundation::Framework *framework)
    {
        framework_ = framework;    
        connected_ = false;
    	
        ///\todo weak_pointerize
        netInterface_ = dynamic_cast<OpenSimProtocol::OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol));
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
	
}

