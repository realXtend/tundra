// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "RexTypes.h"
#include "NetTestLogicModule.h"
#include <Poco/ClassLibrary.h>

#include "RexProtocolMsgIDs.h"
#include "RexUUID.h"
#include "RexLogicModule.h"

#include <iomanip>
#include <limits>
#include <boost/math/special_functions/fpclassify.hpp>

using namespace RexTypes;

namespace 
{
    /// A unary find predicate that looks for a NetOutMessage or a NetInMessage
    /// that has the given desired sequence number in a message pool.
    class InSeqNumMatchPred
    {
    public:
	    InSeqNumMatchPred(uint32_t seqNum):seq_num_(seqNum) {}

	    bool operator()(const std::pair<size_t, NetInMessage> &elem) const
	    {
	        return elem.second.GetSequenceNumber() == seq_num_;
        }

    private:
	    uint32_t seq_num_;
    };

    class OutSeqNumMatchPred
    {
    public:
	    OutSeqNumMatchPred(uint32_t seqNum):seq_num_(seqNum) {}

	    bool operator()(const std::pair<size_t, const NetOutMessage> &elem) const
	    {
	        return elem.second.GetSequenceNumber() == seq_num_;
        }

    private:
	    uint32_t seq_num_;
    };

    const char *VariableTypeToStr(NetVariableType type)
    {
	    const char *data[] = { "Invalid", "U8", "U16", "U32", "U64", "S8", "S16", "S32", "S64", "F32", "F64", "LLVector3", "LLVector3d", "LLVector4",
	                           "LLQuaternion", "UUID", "BOOL", "IPADDR", "IPPORT", "Fixed", "Variable", "BufferByte", "Buffer2Bytes", "Buffer4Bytes" };
	    if (type < 0 || type >= NUMELEMS(data))
		    return data[0];

	    return data[type];
    }
} // ~unnamed namespace

namespace NetTest
{
    NetTestLogicModule::NetTestLogicModule() 
    : ModuleInterfaceImpl("NetTestLogicModule"),
    netTestWindow(0),
    packetDumpWindow(0)
    {
    }
    
    // virtual
    NetTestLogicModule::~NetTestLogicModule()
    {

    }

    void NetTestLogicModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    void NetTestLogicModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void NetTestLogicModule::Initialize()
    {        
        rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
        if (!rexlogic_)
        {
            LogError("Getting RexLogicModule interface did not succeed.");
            return;
        }
        
        LogInfo("Module " + Name() + " initialized.");
    }

    void NetTestLogicModule::PostInitialize()
    {
        //Get event category id's.
        inboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        outboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkOut");
        networkStateCategoryID_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
        
        if (inboundCategoryID_ == 0 || outboundCategoryID_ == 0 || networkStateCategoryID_ == 0)
            LogWarning("Unable to find event category for OpenSimNetwork events!");
            
        InitNetTestWindow();
        InitPacketDumpWindow();
        
        if (netTestWindow || !packetDumpWindow)
        {
            LogError("Could not initialize UI.");
            return;
        }
    }

    void NetTestLogicModule::Uninitialize()
    {
        received_messages_pool_.clear();
        sent_messages_pool_.clear();
            
        SAFE_DELETE(netTestWindow)
        SAFE_DELETE(packetDumpWindow)

        LogInfo("Module " + Name() + " uninitialized.");
    }

    void NetTestLogicModule::Update(Core::f64 frametime)
    {
    }

    bool NetTestLogicModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        if (category_id == networkStateCategoryID_)
        {
            if (event_id == OpenSimProtocol::Events::EVENT_SERVER_CONNECTED)
            {
                if (!netTestWindow)
                    InitNetTestWindow();
                    
                netTestWindow->show();
            }
            if (event_id == OpenSimProtocol::Events::EVENT_SERVER_DISCONNECTED)
                SAFE_DELETE(netTestWindow);
            
            return false;
        }
        else if (category_id == inboundCategoryID_)
        {
            OpenSimProtocol::NetworkEventInboundData *event_data = checked_static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            assert(event_data);
            const NetMessageInfo *info = event_data->message->GetMessageInfo();
            assert(info);
            
		    if(bLogInbound_)
		        WriteToLogWindow(
		            event_data->message->GetSequenceNumber(),
		            info->name,
		            event_data->message->GetDataSize());
            
		    // Save message for debugging purposes.
		    received_messages_pool_.push_back(std::make_pair(event_data->message->GetSequenceNumber(),  *event_data->message));
            
		    // Keep the last 300 inbound messages in memory.
		    if (received_messages_pool_.size() > 300)
		        received_messages_pool_.pop_front();

            switch(event_data->messageID)
		    {
		    case RexNetMsgRegionHandshake:
			    {
                    event_data->message->ResetReading();
                    event_data->message->SkipToNextVariable(); // RegionFlags U32
				    event_data->message->SkipToNextVariable(); // SimAccess U8
				    size_t bytesRead = 0;
				    simName_ = event_data->message->ReadString();

				    std::string title = "Logged in to ";
				    title.append(simName_);
                    netTestWindow->set_title(title);

    			    break;
			    }
		    case RexNetMsgChatFromSimulator:
		        {
		            std::stringstream ss;
		            
                    event_data->message->ResetReading();
		            std::string name = event_data->message->ReadString();
		            event_data->message->SkipToFirstVariableByName("Message");
		            std::string message = event_data->message->ReadString();
		            ss << "[" << Core::GetLocalTimeString() << "] " << name << ": " << message << std::endl;

    	            WriteToChatWindow(ss.str());
		            break;
		        }
		    default:
//			    netInterface_->DumpNetworkMessage(msgID, msg);
			    break;
            }
        }
        else if (category_id == outboundCategoryID_)
        {
            OpenSimProtocol::NetworkEventOutboundData *event_data = checked_static_cast<OpenSimProtocol::NetworkEventOutboundData *>(data);
            assert(event_data);
            const NetMessageInfo *info = event_data->message->GetMessageInfo();
            assert(info);
            
		    if(bLogOutbound_)
		        WriteToLogWindow(
		            event_data->message->GetSequenceNumber(),
		            info->name,
		            event_data->message->BytesFilled(),
		            false);

		    // Save the message for debugging purposes.
		    sent_messages_pool_.push_back(std::make_pair(event_data->message->GetSequenceNumber(), *event_data->message));
		    
		    // Keep the last 200 outbound messages in memory.
		    if (sent_messages_pool_.size() > 200)
		        sent_messages_pool_.pop_front();
        }
        
        return false;
    }

    void NetTestLogicModule::InitNetTestWindow()
    {
        // Create the NetTest UI window from glade (xml) file.
        netTestControls = Gnome::Glade::Xml::create("data/netTestWindow.glade");
        if (!netTestControls)
            return;

        // Set up the log tree view data model and column names.
        Gtk::TreeView *treeview_log = 0;
        netTestControls->get_widget("treeview_log", treeview_log);
        logModel = Gtk::TreeStore::create(logModelColumns);
        treeview_log->set_model(logModel);
        treeview_log->append_column(Glib::ustring("Time"), logModelColumns.colTime);
        treeview_log->append_column(Glib::ustring("In/Out"), logModelColumns.colInOut);
        treeview_log->append_column(Glib::ustring("SeqNum"), logModelColumns.colSeqNum);
        treeview_log->append_column(Glib::ustring("Name"), logModelColumns.colName);
        treeview_log->append_column(Glib::ustring("Size (bytes)"), logModelColumns.colSize);
    
        UpdateLogFilterState();
        
        // Set window title, size and position.
        netTestControls->get_widget("window_nettest", netTestWindow);
        netTestWindow->set_title("NetTest");
        netTestWindow->set_default_size(700, 350);
        netTestWindow->set_position(Gtk::WIN_POS_MOUSE);

        // Bind callbacks.
        netTestControls->connect_clicked("button_chat", sigc::mem_fun(*this, &NetTestLogicModule::OnClickChat));
        netTestControls->connect_clicked("checkbutton_inbound", sigc::mem_fun(*this, &NetTestLogicModule::UpdateLogFilterState));
        netTestControls->connect_clicked("checkbutton_outbound", sigc::mem_fun(*this, &NetTestLogicModule::UpdateLogFilterState));
        Gtk::Entry *entry_chat = netTestControls->get_widget("entry_chat", entry_chat);
        entry_chat->signal_activate().connect(sigc::mem_fun(*this, &NetTestLogicModule::OnClickChat));
        treeview_log->signal_row_activated().connect(sigc::mem_fun(*this, &NetTestLogicModule::OnDoubleClickLog));        
    }
    
    void NetTestLogicModule::InitPacketDumpWindow()
    {
        // Create the NetTest UI window from glade (xml) file.
        packetDumpControls = Gnome::Glade::Xml::create("data/packetDumpWindow.glade");
        if (!packetDumpControls)
            return;
        
        // Set up the packet dump tree view data model and column names.
        Gtk::TreeView *treeview_packetdump = 0;
        packetDumpControls->get_widget("treeview_packetdump", treeview_packetdump);
        packetDumpModel = Gtk::TreeStore::create(packetDumpModelColumns);
        treeview_packetdump->set_model(packetDumpModel);
        treeview_packetdump->append_column(Glib::ustring("Name"), packetDumpModelColumns.colName);
        treeview_packetdump->append_column(Glib::ustring("Data"), packetDumpModelColumns.colData);
        treeview_packetdump->append_column(Glib::ustring("Data Size"), packetDumpModelColumns.colDataSize);
        
        // Set window title, size and position.
        packetDumpControls->get_widget("dialog_packetdump", packetDumpWindow);
        packetDumpWindow->set_title("Packet Dump");
        packetDumpWindow->set_default_size(700, 350);
        packetDumpWindow->set_position(Gtk::WIN_POS_MOUSE);        
    }
    
    void NetTestLogicModule::OnClickChat()
    {
        Gtk::Entry *entry_chat = 0;
        netTestControls->get_widget("entry_chat", entry_chat);
        
        Glib::ustring text = entry_chat->get_text();
        if(text == "")
            return;
        
        rexlogic_->GetServerConnection()->SendChatFromViewerPacket(text);
        entry_chat->set_text("");
    }

    void NetTestLogicModule::OnDoubleClickLog(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn* column)
    {
        Gtk::TreeModel::iterator iter = logModel->get_iter(path);
        if (!iter)
            return;
        
        Gtk::TreeModel::Row row = *iter;
        uint32_t seq_num = row[logModelColumns.colSeqNum];

        if (row[logModelColumns.colInOut] ==  "In")
        {
            ReceivedMessages_t::iterator it = std::find_if(
                received_messages_pool_.begin(),
                received_messages_pool_.end(),
                InSeqNumMatchPred(seq_num));
	        if (it != received_messages_pool_.end())
		    {
                packetDumpWindow->show();
                PopulatePacketDumpTreeView(it->second);
		    }
        }
        else if (row[logModelColumns.colInOut] ==  "Out")
        {
            SentMessages_t::iterator it = std::find_if(
                sent_messages_pool_.begin(),
                sent_messages_pool_.end(),
                OutSeqNumMatchPred(seq_num));
            if (it != sent_messages_pool_.end())
            {
                ///\todo Implement same also for NetOutMessage.
                //packetDumpWindow->show();
                //PopulatePacketDumpTreeView(it->second);
            }
        }
    }
        
    void NetTestLogicModule::WriteToChatWindow(const std::string &message)
    {   
        // Get the widget controls.
		Gtk::TextView *textviewChat = 0;
		netTestControls->get_widget("textview_chat", textviewChat);
		
		// Create text buffer and write data to it.
		Glib::RefPtr<Gtk::TextBuffer> text_buffer = textviewChat->get_buffer();
        Gtk::TextBuffer::iterator iter = text_buffer->get_iter_at_offset(0);
		
		text_buffer->insert(iter, message);
        textviewChat->set_buffer(text_buffer);
    }
    
    void NetTestLogicModule::WriteToLogWindow(uint32_t seq_num, const std::string &name, size_t bytes, bool inbound)
    {
        // Get the widget controls.
		Gtk::TreeView *treeview_log = 0;
        netTestControls->get_widget("treeview_log", treeview_log);
		
        ///\todo Clear the oldest info, if the log window is "full".

        //Fill the TreeView's model.
        Gtk::TreeModel::Row log_row = *(logModel->append());
        log_row[logModelColumns.colTime] = Core::GetLocalTimeString();
        if(inbound)
            log_row[logModelColumns.colInOut] = "In";
        else
            log_row[logModelColumns.colInOut] = "Out";
        log_row[logModelColumns.colSeqNum] = seq_num;
        log_row[logModelColumns.colName] = name;
        log_row[logModelColumns.colSize] = bytes;
        
    }
    
    void NetTestLogicModule::UpdateLogFilterState()
    {
        Gtk::CheckButton *checkbutton_inbound = netTestControls->get_widget("checkbutton_inbound", checkbutton_inbound);
        Gtk::CheckButton *checkbutton_outbound = netTestControls->get_widget("checkbutton_outbound", checkbutton_outbound);
        bLogInbound_ = checkbutton_inbound->get_active();
        bLogOutbound_ = checkbutton_outbound->get_active();
    }
    
    void NetTestLogicModule::PopulatePacketDumpTreeView(NetInMessage msg)
    {
        Gtk::TreeModel::Row row_packet, row_block, row_var; 
        
        const NetMessageInfo *info = msg.GetMessageInfo();
        
        packetDumpModel->clear();
        row_packet = *(packetDumpModel->append());
        
        //Packet name and size.
        row_packet[packetDumpModelColumns.colName] = msg.GetMessageInfo()->name;
        row_packet[packetDumpModelColumns.colDataSize] = msg.GetDataSize();
        
	    int prevBlock = msg.GetCurrentBlock();
	    while(msg.GetCurrentBlock() < msg.GetBlockCount())
	    {
	        // Block
		    size_t curBlock = msg.GetCurrentBlock();
		    const NetMessageBlock &block = info->blocks[curBlock];
		    size_t blockInstanceCount = msg.ReadCurrentBlockInstanceCount();
	        
	        size_t curVar = msg.GetCurrentVariable();
		    const NetMessageVariable &var = block.variables[curVar];
            
            // Add new block row if variable is the first of the block.
            if(curVar == 0)
            {
                row_block = *(packetDumpModel->append(row_packet.children()));
                row_block[packetDumpModelColumns.colName] = block.name;
            }
            
		    prevBlock = curBlock;

		    // Read the variable data.
		    size_t varSize = msg.ReadVariableSize();
		    const uint8_t* data = (const uint8_t*)msg.ReadBytesUnchecked(varSize);
		    msg.SkipToNextVariable(true);
            
            std::stringstream var_data_ss;
		    bool bMalformed = false;
		    if (data && varSize > 0)
		    {
			    for(size_t k = 0; k < varSize && k < 15; ++k) // Print only first 15 bytes of data.
				    var_data_ss << std::hex << (int)data[k] << " ";
                if (varSize > 16)
                    var_data_ss << " ...";
            }
            else
		    {
		        var_data_ss << "Malformed data field!";
			    bMalformed = true;
            }
            
            row_var = *(packetDumpModel->append(row_block.children()));
		    
		    std::stringstream var_ss;
            var_ss << var.name << " (" << VariableTypeToStr(var.type)<< ")";
            row_var[packetDumpModelColumns.colName] = var_ss.str();
                
            // Data and data size.
		    row_var[packetDumpModelColumns.colData] = var_data_ss.str();
		    row_var[packetDumpModelColumns.colDataSize] = varSize;

		    if (bMalformed)
		        return;
	    }
    }
}

using namespace NetTest;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(NetTestLogicModule)
POCO_END_MANIFEST
