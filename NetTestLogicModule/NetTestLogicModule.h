// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetTestLogicModule_h
#define incl_NetTestLogicModule_h

#include "ModuleInterface.h"

#include "OpenSimProtocolModule.h"

#include "NetInMessage.h"
#include "INetMessageListener.h"
#include "NetMessage.h"

namespace Foundation
{
   class Framework;
}

namespace OpenSimProtocol
{
	class OpenSimProtocolModule;
	class RexUUID;
}

/// Object in the sim (prim or avatar)
struct Object
{
	std::string name;
	uint32_t localID;
	RexUUID fullID;
};

/// A unary find predicate that looks for a Object that has the given desired id in a object list container.
class IDMatchPred
{
public:
	IDMatchPred(RexUUID id):rexid_(id) {}
	bool operator()(const std::pair<RexUUID, Object*> &elem) const { return elem.second && elem.second->fullID == rexid_; }

private:
	RexUUID rexid_;
};


namespace NetTest
{
    //! Interface for modules
    class NetTestLogicModule: public Foundation::ModuleInterface_Impl, public INetMessageListener
    {
    public:
        NetTestLogicModule();
        virtual ~NetTestLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();
        
        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
           
        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_NetTest;
        
        /// Called for each network message received.
        virtual void OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg);
        
        typedef std::vector<std::pair<RexUUID, Object*> > ObjectList_t;
		
		/// List of objects (prims) in the world.
		ObjectList_t objectList_;
		
		/// List of avatars (prims) in the world.
		ObjectList_t avatarList_;
		
		/// Name of the sim we're connected.
		std::string simName_;
	    
    private:
        /// Sends the first UDP packet to open up the circuit with the server.
        void SendUseCircuitCodePacket();

        /// Signals taht agent is coming into the region. The region should be expecting the agent.
        /// Server starts to send object updates etc after it has received this packet.
        void SendCompleteAgentMovementPacket();

        /// Sends a message requesting logout from the server. The server is then going to flood us with some
    	/// inventory UUIDs after that, but we'll be ignoring those.
        void SendLogoutRequestPacket();
            
        Foundation::Framework *framework_;

		OpenSimProtocol::OpenSimProtocolModule *netInterface_;

		/// Temporary counter.
		u32 updateCounter;
        
        /// Server-spesific info for this client.
		ClientParameters myInfo_;
		
		/// Signals that the logout message has sent. Do not send anymore messages.
		bool bLogoutSent;		
    };
}
#endif
