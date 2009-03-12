// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "NetTestLogicModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
//#include "EntityInterface.h"

#include "RexProtocolMsgIDs.h"

namespace NetTest
{
    NetTestLogicModule::NetTestLogicModule() : ModuleInterface_Impl(Foundation::Module::Type_NetTest), updateCounter(0)
    {
    }

    NetTestLogicModule::~NetTestLogicModule()
    {
    }

    // virtual
    void NetTestLogicModule::Load()
    {
//        using namespace NetTest;
//        DECLARE_MODULE_EC(EC_Dummy);

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
		netInterface_ = dynamic_cast<OpenSimProtocolModule *>(framework->GetModuleManager()->GetModule("Network")); ///\todo weak_pointerize
		if (!netInterface_)
		{
			LogError("Getting network interface did not succeed.");
			//framework_->GetModuleManager()->UninitializeModule(this);
			return;
		}
		
		//netInterface_->AddListener(this);
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual 
    void NetTestLogicModule::Uninitialize(Foundation::Framework *framework)
    {
		assert(framework_ != NULL);
		framework_ = NULL;

        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void NetTestLogicModule::Update()
    {
        ++updateCounter;
        /*LogInfo("");
        // create new entity
        LogInfo("Constructing entity with component: " + Test::EC_Dummy::Name() + ".");

        Foundation::SceneServiceInterface *scene = framework_->GetService<Foundation::SceneServiceInterface>(Foundation::Service::ST_Scene);
        Foundation::EntityPtr entity = scene->CreateEntity();
        assert (entity.get() != 0 && "Failed to create entity.");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::Name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->AddEntityComponent(component);
        component = entity->GetComponent(component->_Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());

        framework_->Exit();
        assert (framework_->IsExiting());

        LogInfo("");*/
    }
    
    //virtual 
    void NetTestLogicModule::OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg)
    {
    	switch(msgID)
		{
		case RexNetMsgRegionHandshake:
			{
				/*std::cout << "\"RegionHandshake\" received, " << msg->GetDataSize() << " bytes." << std::endl;
				msg->SkipToNextVariable(); // RegionFlags U32
				msg->SkipToNextVariable(); // SimAccess U8
				size_t bytesRead = 0;
				const char* simName = (const char* )msg->ReadBuffer(&bytesRead);
				
				/*for(size_t i = 0; i < 15; ++i)
				{
					std::cout << i << " " << msg->CheckNextVariableType() << std::endl;
					msg->SkipToNextVariable();
				}
				
				UUID td3 = msg->ReadUUID();
				std::cout << "\"" << td3.ToString() << "\"" << std::endl;

				for(size_t i = 0; i < 8; ++i)
				{
					std::cout << msg->ReadF32() << std::endl;
				}

				myRegionID = msg->ReadUUID();
				std::cout << myRegionID.ToString() << std::endl;*/
				//myRegionID = UUID("0000-0000-0000-0000");
				break;
			}
		case RexNetMsgObjectUpdate:
			{
			    
				/*std::cout << "\"ObjectUpdate\" received, " << msg->GetDataSize() << " bytes." << std::endl;
				Object obj;
				msg->SkipToNextVariable();		// RegionHandle U64
				msg->SkipToNextVariable();		// TimeDilation U16
				obj.localID = msg->ReadU32(); 
				msg->SkipToNextVariable();		// State U8
				obj.fullID = msg->ReadUUID();
				msg->SkipToNextVariable();		// CRC U32
				uint8_t PCode = msg->ReadU8();

				//std::list<Object>::iterator it;
				//it = std::find(objectList.begin(), objectList.end(), obj);
				//if (it == objectList.end())
				{
					// If prim, PCode is 0x09;
					if (PCode == 0x09)
						objectList.push_back(obj);
					// If avatar, PCode is 0x2f;
					else if (PCode == 0x2f)
					{
						
						msg->SkipToFirstVariableByName("NameValue");
						size_t bytesRead = 0;
						const char *nameData = (const char *)msg->ReadBuffer(&bytesRead);
						string name(nameData);
						string first = "FirstName STRING RW SV ";
						string last = "LastName STRING RW SV ";
						size_t pos;

						pos = name.find(first);
						name.replace(pos, strlen(first.c_str()), "");
						pos = name.find(last);
						name.replace(pos, strlen(last.c_str()), "");
						pos = name.find("\n");
						name.replace(pos, 1, " ");
						obj.name = name;
						objectList.push_back(obj);
						std::cout << "Avatar \"" << name << "\" joined the sim" << std::endl;
					}
				}*/
				break;
			}
		case RexNetMsgLogoutReply:
			{
				//RexUUID aID = msg->ReadUUID();
				//RexUUID sID = msg->ReadUUID();
	
				/*
				size_t blockCount = msg.ReadBlockCount();
				for(int i = 0; i < blockCount; ++i)
				{
					UUID itemID = msg.ReadUUID();
					size_t numBytes = 0;//msg.ReadVariableSize();
					const uint8_t *NetInMessage::ReadVariableLengthVar(size_t *numBytes) const;
					uint8_t *data = msg.ReadVariableLengthVar(&numBytes);
					// Access data.

				}*/
				
				// Logout if the id's match.
				/*if (aID == myAgentID && sID == mySessionID)
				{
					cout << "\"LogoutReply\" received with matching IDs. Quitting!" << endl;
					bRunning = false;
				}*/
				break;
			}
		default:
			//netInterface_->DumpNetworkMessage(msgID, msg);
			break;
		}        
    }
}

using namespace NetTest;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(NetTestLogicModule)
POCO_END_MANIFEST
