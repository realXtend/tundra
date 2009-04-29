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

#include "Foundation.h"
#include "ModuleInterface.h"

#include "OpenSimProtocolModule.h"
#include "NetInMessage.h"
#include "NetMessage.h"

namespace RexLogic
{
    class RexLogicModule;
}

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
    class NetTestLogicModule: public Foundation::ModuleInterfaceImpl
    {
    public:
        NetTestLogicModule();
        virtual ~NetTestLogicModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(Core::f64 frametime);
        
        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);
            
        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
           
        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_NetTestLogic;
        
        /// Initializes the NetTest window.
        void InitNetTestWindow();
        
        /// Initializes the Packet Dump window.
        void InitPacketDumpWindow();

        /// Callback function for the 'Chat' button. Sends UPD chat message.
        void OnClickChat();
        
        /// Callback function for double-click on the logging treeview. Opens the activated message in a new window.
        void OnDoubleClickLog(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);        
        
        /// Writes a message to the chat window.
        void WriteToChatWindow(const std::string &message);
        
        /// Writes a message to the log window.
        void WriteToLogWindow(uint32_t seq_num, const std::string &name, size_t bytes, bool inbound = true);
        
        /// Update which messages are shown in the log window.
        void UpdateLogFilterState();
        
        /// Populates the Packet Dump Tree view window.
        void PopulatePacketDumpTreeView(NetInMessage msg);
        void PopulatePacketDumpTreeView(NetOutMessage *msg);
        
		/// Name of the sim we're connected.
		std::string simName_;
		
        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> netTestControls;

        // Handle to the packet dump window controls.
        Glib::RefPtr<Gnome::Glade::Xml> packetDumpControls;

        // The GTK window for NetTest UI.
        Gtk::Window *netTestWindow;

        // The GTK window for packet dump UI.
        Gtk::Window *packetDumpWindow;
            
        /// Tree model columns for entity list.
        Glib::RefPtr<Gtk::TreeStore> logModel;
        
        /// Gtk treeview data model for the in/outbound packet log window.
        class LogModelColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            LogModelColumns()
            {
                add(colTime);
                add(colInOut);
                add(colSeqNum);
                add(colName);
                add(colSize);
            }
            Gtk::TreeModelColumn<Glib::ustring> colTime;
            Gtk::TreeModelColumn<Glib::ustring> colInOut;
            Gtk::TreeModelColumn<uint32_t> colSeqNum;
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<size_t> colSize;
        };
            
        const LogModelColumns logModelColumns;

        /// Tree model columns for packet dump.
        Glib::RefPtr<Gtk::TreeStore> packetDumpModel;
        
        /// Gtk treeview data model for the packet contents dump window.
        class PacketDumpModelColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            PacketDumpModelColumns()
            {
                add(colName);
                add(colData);
                add(colDataSize);
            }
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<Glib::ustring> colData;
            Gtk::TreeModelColumn<size_t> colDataSize;
        };
            
        const PacketDumpModelColumns packetDumpModelColumns;
        
    private:
        void operator=(const NetTestLogicModule &);
        NetTestLogicModule(const NetTestLogicModule &);
               
        /// Clears the message pools after a logout and closes the NetTest window.
        void LogOut();
        
        // Pointer to rexlogic interface
        RexLogic::RexLogicModule *rexlogic_;

		/// Show inbound messages in the log.
		bool bLogInbound_;
		
		/// Show outbound messages in the log.
		bool bLogOutbound_;
		
		/// Category id for incoming messages.
		Core::event_category_id_t inboundCategoryID_;
		
		/// Category id for incoming messages.
		Core::event_category_id_t outboundCategoryID_;
		
		/// Category id for network state messages.
		Core::event_category_id_t networkStateCategoryID_;
	    
	    /// A pool of reveceived NetInMessage structures. Used for debugging.
	    typedef std::list<std::pair<uint32_t, NetInMessage> > ReceivedMessages_t;
	    ReceivedMessages_t received_messages_pool_;
	    
	    /// A pool of sent NetOutMessage structures. Used for debugging.
	    typedef std::list<std::pair<uint32_t, NetOutMessage> > SentMessages_t;
	    SentMessages_t sent_messages_pool_;

    };
}
#endif
