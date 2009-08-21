// For conditions of distribution and use, see copyright notice in license.txt

/// @file XMLRPCLoginThread.cpp
/// @brief

#include "StableHeaders.h"

#include <boost/shared_ptr.hpp>
#include <utility>
#include <algorithm>

#include "XMLRPCLoginThread.h"
#include "OpenSimProtocolModule.h"
#include "XMLRPCEPI.h"
#include "md5wrapper.h"
#include "OpenSimAuth.h"
#include "Inventory.h"

namespace OpenSimProtocol
{

XMLRPCLoginThread::XMLRPCLoginThread() : beginLogin_(false), ready_(false)
{
}

XMLRPCLoginThread::~XMLRPCLoginThread()
{
}

void XMLRPCLoginThread::operator()()
{
    if(beginLogin_)
    {   
        threadState_->state = Connection::STATE_WAITING_FOR_XMLRPC_REPLY;
        
        bool success = PerformXMLRPCLogin();
        if (success && !authentication_)
        {
            // Login without authentication succeeded.
            threadState_->state = Connection::STATE_XMLRPC_REPLY_RECEIVED;
        }
        else if (success && authentication_)
        {
            // First round of authentication succeeded; sessions hash, grid & avatar url's reveiced.
            threadState_->state = Connection::STATE_XMLRPC_AUTH_REPLY_RECEIVED;
            
            // Perform second round to received the agent, session & region id's.
            callMethod_ = "login_to_simulator";
            
            bool success2 = PerformXMLRPCLogin();
            if (success2)
                threadState_->state = Connection::STATE_XMLRPC_REPLY_RECEIVED;
            else
                threadState_->state = Connection::STATE_LOGIN_FAILED;
        }
        else
            threadState_->state = Connection::STATE_LOGIN_FAILED;
        
        beginLogin_ = false;
    }
}

volatile Connection::State XMLRPCLoginThread::GetState() const
{
    if (!ready_)
        return Connection::STATE_DISCONNECTED;
    else 
        return threadState_->state;
}

void XMLRPCLoginThread::SetupXMLRPCLogin(
            const std::string& first_name, 
			const std::string& last_name, 
			const std::string& password,
			const std::string& worldAddress,
			const std::string& worldPort,
			const std::string& callMethod,
            ConnectionThreadState *thread_state,
			const std::string& authentication_login,
			const std::string& authentication_address,
			const std::string& authentication_port,
			bool authentication)
{
    // Save the info for login.
    firstName_ = first_name;
    lastName_ = last_name;
    password_ = password;
    worldAddress_ = worldAddress;
    worldPort_ = worldPort;
    callMethod_ = callMethod;
    authenticationLogin_ = authentication_login;
    authenticationAddress_ = authentication_address;
    authenticationPort_ = authentication_port;
    authentication_ = authentication,
    threadState_ = thread_state;
    
    ready_ = true;
    
    threadState_->state = Connection::STATE_INIT_XMLRPC;
    
    beginLogin_ = true;
}

/// This function reads the address to connect to for the simulation UDP connection.
/// @param call Pass in the object to a XMLRPCEPI call that has already been performed. Only the reply part
///     will be read by this function.
/// @return The ip:port to connect to with the UDP socket, or "" if there was an error.
static std::string ExtractGridAddressFromXMLRPCReply(XMLRPCEPI &call)
{
    std::string gridUrl = call.GetReply<std::string>("sim_ip");
    if (gridUrl.size() == 0)
        return "";

    int port = call.GetReply<int>("sim_port");
    if (port <= 0 || port >= 65536)
        return "";

    std::string s;
    std::stringstream out;
    out << gridUrl << ":" << port;

    return out.str();
}

/// This function reads the inventory tree that was stored in the XMLRPC login_to_simulator reply.
/// @param call Pass in the object to a XMLRPCEPI call that has already been performed. Only the reply part
///     will be read by this function.
/// @return The inventory object, or null pointer if an error occurred.
boost::shared_ptr<Inventory> ExtractInventoryFromXMLRPCReply(XMLRPCEPI &call)
{
    boost::shared_ptr<Inventory> inventory = boost::shared_ptr<Inventory>(new Inventory);
    XMLRPCCall *xmlrpcCall = call.GetXMLRPCCall();
    if (!xmlrpcCall)
        throw XMLRPCException("Failed to read inventory, no XMLRPC Reply to read!");
    XMLRPC_REQUEST request = xmlrpcCall->GetReply();
    if (!request)
        throw XMLRPCException("Failed to read inventory, no XMLRPC Reply to read!");

    XMLRPC_VALUE result = XMLRPC_RequestGetData(request);
    if (!result)
        throw XMLRPCException("Failed to read inventory, the XMLRPC Reply did not contain any data!");

    XMLRPC_VALUE inventoryNode = XMLRPC_VectorGetValueWithID(result, "inventory-skeleton");

    if (!inventoryNode || XMLRPC_GetValueType(inventoryNode) != xmlrpc_vector)
        throw XMLRPCException("Failed to read inventory, inventory-skeleton in the reply was not properly formed!");
    
    typedef std::pair<RexUUID, InventoryFolder> DetachedInventoryFolder;
    typedef std::list<DetachedInventoryFolder> DetachedInventoryFolderList;
    DetachedInventoryFolderList folders;
    
    XMLRPC_VALUE item = XMLRPC_VectorRewind(inventoryNode);
    while(item)
    {
        const char *id = XMLRPC_GetValueID(item);
        if (id)
            std::cout << id << std::endl;
        XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(item);
        if (type == xmlrpc_vector) // xmlrpc-epi handles structs as arrays.
        {
            DetachedInventoryFolder folder;

            XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(item, "name");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.name = XMLRPC_GetValueString(val);

            val = XMLRPC_VectorGetValueWithID(item, "parent_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.first.FromString(XMLRPC_GetValueString(val));

            val = XMLRPC_VectorGetValueWithID(item, "version");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.version = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "type_default");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_int)
                folder.second.type_default = XMLRPC_GetValueInt(val);

            val = XMLRPC_VectorGetValueWithID(item, "folder_id");
            if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                folder.second.id.FromString(XMLRPC_GetValueString(val));

            folders.push_back(folder);
        }

        item = XMLRPC_VectorNext(inventoryNode);
    }

    // Find and set the inventory root folder.
    XMLRPC_VALUE inventoryRootNode = XMLRPC_VectorGetValueWithID(result, "inventory-root");
    if (!inventoryRootNode)
        throw XMLRPCException("Failed to read inventory, inventory-root in the reply was not present!");
    if (!inventoryRootNode || XMLRPC_GetValueType(inventoryRootNode) != xmlrpc_vector)
        throw XMLRPCException("Failed to read inventory, inventory-root in the reply was not properly formed!");
    XMLRPC_VALUE inventoryRootNodeFirstElem = XMLRPC_VectorRewind(inventoryRootNode);
    if (!inventoryRootNodeFirstElem || XMLRPC_GetValueType(inventoryRootNodeFirstElem) != xmlrpc_vector)
        throw XMLRPCException("Failed to read inventory, inventory-root in the reply was not properly formed!");
    XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(inventoryRootNodeFirstElem, "folder_id");
    if (!val || XMLRPC_GetValueType(val) != xmlrpc_string)
        throw XMLRPCException("Failed to read inventory, inventory-root struct value folder_id not present!");
    RexUUID inventoryRootFolderID(XMLRPC_GetValueString(val));
    if (inventoryRootFolderID.IsNull())
        throw XMLRPCException("Failed to read inventory, inventory-root value folder_id was null or unparseable!");

    for(DetachedInventoryFolderList::iterator iter = folders.begin(); iter != folders.end(); ++iter)
        if (iter->second.id == inventoryRootFolderID)
        {
            inventory->root = iter->second;
            folders.erase(iter);
            break;
        }
    if (inventory->root.id.IsNull())
        throw XMLRPCException("Failed to read inventory, inventory-root value folder_id pointed to a nonexisting folder!");
    
    bool progress = true;
    while(folders.size() > 0 && progress)
    {   
        progress = false;
        DetachedInventoryFolderList::iterator iter = folders.begin();
        while(iter != folders.end())
        {
            DetachedInventoryFolderList::iterator next = iter;
            ++next;

            InventoryFolder *parent = inventory->GetFirstSubFolderByID(iter->first);
            if (parent)
            {
                parent->AddSubFolder(iter->second);
                progress = true;
                folders.erase(iter);
            }
            iter = next;
        }
    }

    inventory->DebugDumpInventoryFolderStructure();

    return inventory;
}

bool XMLRPCLoginThread::PerformXMLRPCLogin()
{
    // create a MD5 hash for the password, MAC address and HDD serial number.
    std::string mac_addr = GetMACaddressString();
    std::string id0 = GetId0String();

    md5wrapper md5;
    std::string password_hash = "$1$" + md5.getHashFromString(password_);
    std::string mac_hash = md5.getHashFromString(mac_addr);
    std::string id0_hash = md5.getHashFromString(id0);
    
    XMLRPCEPI call;
    try
    {
        if (authentication_ && callMethod_ == "ClientAuthentication" )
        {
            call.Connect(authenticationAddress_, authenticationPort_);
            call.CreateCall(callMethod_);
        }
        else
        {
            call.Connect(worldAddress_, worldPort_);
            call.CreateCall(callMethod_);
        }
    }
    catch (XMLRPCException& ex)
    {
        // Initialisation error.
        OpenSimProtocolModule::LogError(ex.what());
        return false;
    }
    
    try
    {
        if (!authentication_ && callMethod_ == std::string("login_to_simulator"))
        {
            // We're performing a login in OpenSim style, no external authentication server of any kind.
            call.AddMember("first", firstName_);
            call.AddMember("last", lastName_);
            call.AddMember("passwd", password_hash);
         }
        else if (authentication_ && callMethod_ == std::string("ClientAuthentication"))
        {
            // We're performing an authentication method call to the Rex auth server. This is the first stage of
            // Rex login.
            std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" +authenticationPort_; 
            call.AddMember("account", account);
            call.AddMember("passwd", password_hash);
            std::string loginuri = "";

            loginuri = loginuri+worldAddress_+":"+ worldPort_;
            call.AddMember("loginuri", loginuri);
        }
        else if (authentication_ && callMethod_ == std::string("login_to_simulator") )
        {
            // We're logging in to the sim server after having authenticated with the Rex auth server first.
            // This is the second stage of Rex login.
            call.AddMember("sessionhash", threadState_->parameters.sessionHash);

            std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" + authenticationPort_; 
            call.AddMember("account", account);
            
            // It seems that when connecting to a local authentication grid, firstname, lastname and password are
            // needed, even though they were not supposed to.
            call.AddMember("first", firstName_);
            call.AddMember("last", lastName_);
            call.AddMember("passwd", password_hash);

            std::string address = authenticationAddress_ + ":" + authenticationPort_;
            call.AddMember("AuthenticationAddress", address);
            std::string loginuri = "";
            if (!worldAddress_.find("http") != std::string::npos )
                loginuri = "http://";

            if ( authenticationLogin_ == std::string("openid") )
            {
                loginuri = loginuri + worldAddress_;
            } else
            {
                loginuri = loginuri + worldAddress_ + ":" + worldPort_;
            }

            call.AddMember("loginuri", loginuri.c_str());
        }

        call.AddMember("start", std::string("last")); // Starting position: last/home
        call.AddMember("version", std::string("realXtend Naali 0.0.1"));  ///\todo Make build system create versioning information.
        call.AddMember("channel", std::string("realXtend"));
        call.AddMember("platform", std::string("Win")); ///\todo.
        call.AddMember("mac", mac_hash);
        call.AddMember("id0", id0_hash);
        call.AddMember("last_exec_event", int(0)); // ?

        // The contents of 'options' array unknown. ///\todo Go through them and identify what they really affect.
        std::string arr = "options";
        call.AddStringToArray(arr, "inventory-root");
        call.AddStringToArray(arr, "inventory-skeleton");
        call.AddStringToArray(arr, "inventory-lib-root");
        call.AddStringToArray(arr, "inventory-lib-owner");
        call.AddStringToArray(arr, "inventory-skel-lib");
        call.AddStringToArray(arr, "initial-outfit");
        call.AddStringToArray(arr, "gestures");
        call.AddStringToArray(arr, "event_categories");
        call.AddStringToArray(arr, "event_notifications");
        call.AddStringToArray(arr, "classified_categories");
        call.AddStringToArray(arr, "buddy-list");
        call.AddStringToArray(arr, "ui-config");
        call.AddStringToArray(arr, "tutorial_setting");
        call.AddStringToArray(arr, "login-flags");
        call.AddStringToArray(arr, "global-textures");
    }
    catch (XMLRPCException& ex)
    {
        // Initialisation error.
        OpenSimProtocolModule::LogError(ex.what());
        return false;
    }

    try
    {
        call.Send();
    }
    catch(XMLRPCException& ex)
    {
        //Send error
        OpenSimProtocolModule::LogError(ex.what());
        return false;
    }

    try
    {
	    if (!authentication_)
	    {
            threadState_->parameters.sessionID.FromString(call.GetReply<std::string>("session_id"));
            threadState_->parameters.agentID.FromString(call.GetReply<std::string>("agent_id"));
		    threadState_->parameters.circuitCode = call.GetReply<int>("circuit_code");
            threadState_->parameters.seedCapabilities = call.GetReply<std::string>("seed_capability");
            
            threadState_->parameters.gridUrl = ExtractGridAddressFromXMLRPCReply(call);
            if (threadState_->parameters.gridUrl.size() == 0)
                throw XMLRPCException("Failed to extract sim_ip and sim_port from login_to_simulator reply!");

            if (threadState_->parameters.sessionID.ToString() == std::string("") ||
                threadState_->parameters.agentID.ToString() == std::string("") ||
                threadState_->parameters.circuitCode == 0)
                throw XMLRPCException("Failed to receive sessionID, agentID or circuitCode from login_to_simulator reply!");

            threadState_->parameters.inventory = ExtractInventoryFromXMLRPCReply(call);
	    }
	    else if (authentication_ && callMethod_ != std::string("login_to_simulator")) 
	    {
		    // Authentication results
            threadState_->parameters.sessionHash = call.GetReply<std::string>("sessionHash");
            threadState_->parameters.gridUrl = std::string(call.GetReply<std::string>("gridUrl"));
			//\bug the grid url provided by authentication server points to tcp port, but the grid url is used in the code to connect to udp port
            threadState_->parameters.avatarStorageUrl = std::string(call.GetReply<std::string>("avatarStorageUrl"));
	    }
	    else if (authentication_ && callMethod_ == std::string("login_to_simulator"))
	    {
            threadState_->parameters.sessionID.FromString(call.GetReply<std::string>("session_id"));
            threadState_->parameters.agentID.FromString(call.GetReply<std::string>("agent_id"));
		    threadState_->parameters.circuitCode = call.GetReply<int>("circuit_code");
            threadState_->parameters.seedCapabilities = call.GetReply<std::string>("seed_capability");

			///\bug related to one 10 lines above. instead of using port defined in authentication server, 
            /// use the one given by simulator.
            /// Does this still apply? -jj. Is this a bug in the rex auth server? If so, flag as a workaround or something similar.
            threadState_->parameters.gridUrl = ExtractGridAddressFromXMLRPCReply(call);
            if (threadState_->parameters.gridUrl.size() == 0)
                throw XMLRPCException("Failed to extract sim_ip and sim_port from login_to_simulator reply!");

            threadState_->parameters.inventory = ExtractInventoryFromXMLRPCReply(call);
	    }
        else
            throw XMLRPCException(std::string("Undefined login method ") + callMethod_ + " in XMLRPCLoginThread!");
    }
    catch(XMLRPCException& ex)
    {
        OpenSimProtocolModule::LogError(std::string("Login procedure threw a XMLRPCException, reason: \"") + ex.what() + std::string("\"."));

        // Read error message from reply
        try
        {
            ///\todo transfer error message to login screen. 
            threadState_->errorMessage = call.GetReply<std::string>("message");
            OpenSimProtocolModule::LogError(std::string("login_to_simulator reply returned the error message \"") + threadState_->errorMessage + std::string("\"."));
        }
        catch(XMLRPCException &ex)
        {
            OpenSimProtocolModule::LogError(std::string("login_to_simulator reply did not contain an error message (") + ex.what() + std::string(")."));
        }
        
        return false;
    }
	   
	return true;
}

}
