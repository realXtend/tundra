// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetTestLogicModule_h
#define incl_NetTestLogicModule_h

#pragma warning( push )
#pragma warning( disable : 4250 )
#undef max
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

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
        
        /// Initializes the Login window.
		void InitLoginWindow();
        
        /// Initializes the Login window.
        void InitNetTestWindow();
        
        /// Connects to server.
        void OnClickConnect();
        
        /// Disconnects from the server.
        void OnClickLogout();
        
        /// Terminates the application.
        void OnClickQuit();
        
        /// Chat.
        void OnClickChat();
        
        /// Writes a message to the chat window.
        void WriteToChatWindow(const std::string &message);
        
        /// Writes a message to the log window.
        void WriteToLogWindow(const std::string &message);
        
        /// Type definition for object lists.
        typedef std::vector<std::pair<RexUUID, Object*> > ObjectList_t;
		
		/// List of objects (prims) in the world.
		ObjectList_t objectList_;
		
		/// List of avatars (prims) in the world.
		ObjectList_t avatarList_;
		
		/// Name of the sim we're connected.
		std::string simName_;
		
        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> loginControls;

        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> netTestControls;

        // The GTK windows, entry fields and buttons.
        Gtk::Window *loginWindow;
        Gtk::Window *netTestWindow;

    private:
        void operator=(const NetTestLogicModule &);
        NetTestLogicModule(const NetTestLogicModule &);

        /// Sends the first UDP packet to open up the circuit with the server.
        void SendUseCircuitCodePacket();

        /// Signals taht agent is coming into the region. The region should be expecting the agent.
        /// Server starts to send object updates etc after it has received this packet.
        void SendCompleteAgentMovementPacket();
	    
	    // Send the UDP chat packet.
	    void SendChatFromViewerPacket(const char *text);
        
        /// Sends a message requesting logout from the server. The server is then going to flood us with some
    	/// inventory UUIDs after that, but we'll be ignoring those.
        void SendLogoutRequestPacket();
        
        /// Pointer to the main framework.
        Foundation::Framework *framework_;
        
        /// Pointer to the network interface.
		OpenSimProtocol::OpenSimProtocolModule *netInterface_;

        /// Server-spesific info for this client.
		ClientParameters myInfo_;
		
		/// Signals that NetTestModule running.
		bool bRunning_;

		/// Signals that the logout message has sent. Do not send anymore messages.
		bool bLogoutSent_;
    };
}
#endif
