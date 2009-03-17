// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "NetTestLogicModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
//#include "EntityInterface.h"

#include "RexProtocolMsgIDs.h"

/// Login credentials. 
///\todo Use real ones, not hardcoded.
const char serverAddress[] = "192.168.1.144";
const int port = 9000;
const char firstName[] = "jj";
const char lastName[] = "jj";
const char password[] = "jj";

namespace NetTest
{
    NetTestLogicModule::NetTestLogicModule() 
    : ModuleInterface_Impl(Foundation::Module::MT_NetTest), updateCounter(0), bLogoutSent(false)
    {
        objectList_.clear();
        avatarList_.clear();
    }

    NetTestLogicModule::~NetTestLogicModule()
    {
    	for(ObjectList_t::iterator iter = objectList_.begin(); iter != objectList_.end(); ++iter)
		    delete iter->second;

    	for(ObjectList_t::iterator iter = avatarList_.begin(); iter != avatarList_.end(); ++iter)
		    delete iter->second;
    }

    // virtual
    void NetTestLogicModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void NetTestLogicModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void NetTestLogicModule::Initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        framework_ = framework;
		
		using namespace OpenSimProtocol;
		///\todo weak_pointerize
		netInterface_ = dynamic_cast<OpenSimProtocolModule *>(framework->GetModuleManager()->GetModule("Network"));
		if (!netInterface_)
		{
			LogError("Getting network interface did not succeed.");
			//framework_->GetModuleManager()->UninitializeModule(this);
			return;
		}
		
		netInterface_->AddListener(this);
		
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void NetTestLogicModule::Uninitialize(Foundation::Framework *framework)
    {
		assert(framework_ != NULL);
		framework_ = NULL;
        
        ///\todo Unregister from OpenSimProtocolModule.
        
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void NetTestLogicModule::Update()
    {
        if (bLogoutSent)
            return;
        
        ///\todo Use time/frame count or some other better metric.
        ++updateCounter;
        
        if(updateCounter == 150)
        {
            bool success = netInterface_->ConnectToRexServer(firstName, lastName, password, serverAddress, port, &myInfo_);
            if(success)
            {
                SendUseCircuitCodePacket();
                SendCompleteAgentMovementPacket();
                LogInfo("Connected to server " + std::string(serverAddress) + ".");
            }
            else
                LogError("Connecting to server " + std::string(serverAddress) + " failed.");
        }
        
        if(updateCounter == 20000)
        {
            SendLogoutRequestPacket();
            bLogoutSent = true;
        }
    }
    
    //virtual 
    void NetTestLogicModule::OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg)
    {
    	switch(msgID)
		{
		case RexNetMsgRegionHandshake:
			{
				LogInfo("\"RegionHandshake\" received, " + TO_STRING(msg->GetDataSize()) + " bytes.");
				msg->SkipToNextVariable(); // RegionFlags U32
				msg->SkipToNextVariable(); // SimAccess U8
				size_t bytesRead = 0;
				simName_ = (const char *)msg->ReadBuffer(&bytesRead);
				LogInfo("Joined to the sim \"" + simName_ + "\".");
    			break;
			}
		case RexNetMsgObjectUpdate:
			{
				LogInfo("\"ObjectUpdate\" received, " + TO_STRING(msg->GetDataSize()) + " bytes.");
				
				Object *obj = new Object;
				msg->SkipToNextVariable();		// RegionHandle U64
				msg->SkipToNextVariable();		// TimeDilation U16
				obj->localID = msg->ReadU32(); 
				msg->SkipToNextVariable();		// State U8
				obj->fullID = msg->ReadUUID();
				msg->SkipToNextVariable();		// CRC U32
				uint8_t PCode = msg->ReadU8();
				
				ObjectList_t::iterator it;
    			if (PCode == 0x09)
				{
				    // If prim, PCode is 0x09.
				    it = std::find_if(objectList_.begin(), objectList_.end(), IDMatchPred(obj->fullID));
				    if (it != objectList_.end())
				    {
				        SAFE_DELETE(obj);
				        return;
				    }
				    else
				        objectList_.push_back(std::make_pair(obj->fullID, obj));
                }
				else if (PCode == 0x2f)
				{
				    // If avatar, PCode is 0x2f.
				    it = std::find_if(avatarList_.begin(), avatarList_.end(), IDMatchPred(obj->fullID));
				    if (it != avatarList_.end())
				        return;
				    
				    // Read the name of the avatar.
				    msg->SkipToFirstVariableByName("NameValue");
					size_t bytesRead = 0;
					std::string name = (const char *)msg->ReadBuffer(&bytesRead);
    					
					// Parse the name.
					std::string first = "FirstName STRING RW SV ";
					std::string last = "LastName STRING RW SV ";
					size_t pos;

					pos = name.find(first);
					name.replace(pos, strlen(first.c_str()), "");
					pos = name.find(last);
					name.replace(pos, strlen(last.c_str()), "");
					pos = name.find("\n");
					name.replace(pos, 1, " ");
					obj->name = name;
					avatarList_.push_back(std::make_pair(obj->fullID, obj));
					
					LogInfo("Avatar \"" + name + "\" joined the sim");
                }
                else
                    //We're not interested in any other objects at the moment.
                    if(obj)
                        SAFE_DELETE(obj);
                    
				break;
			}
		case RexNetMsgLogoutReply:
			{
			    LogInfo("\"LogoutReply\" received, " + TO_STRING(msg->GetDataSize()) + " bytes.");
				RexUUID aID = msg->ReadUUID();
				RexUUID sID = msg->ReadUUID();
	
				/*size_t blockCount = msg.ReadBlockCount();
				for(int i = 0; i < blockCount; ++i)
				{
					UUID itemID = msg.ReadUUID();
					size_t numBytes = 0;//msg.ReadVariableSize();
					const uint8_t *NetInMessage::ReadVariableLengthVar(size_t *numBytes) const;
					uint8_t *data = msg.ReadVariableLengthVar(&numBytes);
					// Access data.

				}*/
				
				// Logout if the id's match.
				if (aID == myInfo_.agentID && sID == myInfo_.sessionID)
				{
					LogInfo("\"LogoutReply\" received with matching IDs. Quitting!");
                    framework_->Exit();
                    assert (framework_->IsExiting());
				}
				break;
			}
		default:
			netInterface_->DumpNetworkMessage(msgID, msg);
			break;
		}        
    }
    
	void NetTestLogicModule::SendUseCircuitCodePacket()
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgUseCircuitCode);
		assert(m);
		m->AddU32(myInfo_.circuitCode);
		m->AddUUID(myInfo_.sessionID);
		m->AddUUID(myInfo_.agentID);
		netInterface_->FinishMessageBuilding(m);
	}

    void NetTestLogicModule::SendCompleteAgentMovementPacket()
    {
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgCompleteAgentMovement);
	    assert(m);
	    m->AddUUID(myInfo_.agentID);
	    m->AddUUID(myInfo_.sessionID);
	    m->AddU32(myInfo_.circuitCode);
	    netInterface_->FinishMessageBuilding(m);
    }
    
	void NetTestLogicModule::SendLogoutRequestPacket()
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgLogoutRequest);
		assert(m);
    	m->AddUUID(myInfo_.agentID);
		m->AddUUID(myInfo_.sessionID);
	    netInterface_->FinishMessageBuilding(m);
	}
}

using namespace NetTest;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(NetTestLogicModule)
POCO_END_MANIFEST
