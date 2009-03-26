// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetTestLogicModule_h
#define incl_NetTestLogicModule_h

#undef NETTESTLOGIC_MODULE_API
#if defined (_WINDOWS)
#if defined(NETTESTLOGIC_MODULE_EXPORTS) 
#define NETTESTLOGIC_MODULE_API __declspec(dllexport) 
#else
#define NETTESTLOGIC_MODULE_API __declspec(dllimport) 
#endif
#endif

#ifndef NETTESTLOGIC_MODULE_API
#define NETTESTLOGIC_MODULE_API
#endif

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

#include "Foundation.h"
#include "ModuleInterface.h"

#include "OpenSimProtocolModule.h"
#include "NetInMessage.h"
#include "NetMessage.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

namespace NetTest
{
    //! Interface for modules
    class NetTestLogicModule: public Foundation::ModuleInterface_Impl
    {
    public:
        NetTestLogicModule();
        virtual ~NetTestLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update();
        
        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);
            
        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
           
        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_NetTestLogic;

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
        
        /// Update which messages are shown in the log window.
        void UpdateLogFilterState();
        
		/// Name of the sim we're connected.
		std::string simName_;
		
        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> loginControls;

        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> netTestControls;

        // The GTK window for login UI.
        Gtk::Window *loginWindow;
        
        // The GTK window for NetTest UI.
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
        
        /// Pointer to the network interface.
		OpenSimProtocol::OpenSimProtocolModule *netInterface_;

        /// Server-spesific info for this client.
		ClientParameters myInfo_;
		
		/// Signals that NetTestModule running.
		bool bRunning_;

		/// Signals that the logout message has sent. Do not send anymore messages.
		bool bLogoutSent_;
		
		/// Show inbound messages in the log.
		bool bLogInbound_;
		
		/// Show outbound messages in the log.
		bool bLogOutbound_;
		
		/// Category id for incoming messages.
		Core::event_category_id_t inboundCategoryID_;
		
		/// Category id for incoming messages.
		Core::event_category_id_t outboundCategoryID_;
    };
}
#endif
