// For conditions of distribution and use, see copyright notice in license.txt

/// @file XMLRPCLoginThread.cpp
/// @brief

#include <boost/shared_ptr.hpp>

#include "StableHeaders.h"

#include "XMLRPCLoginThread.h"
#include "OpenSimProtocolModule.h"
#include "PocoXMLRPC.h"
#include "md5wrapper.h"
#include "OpenSimAuth.h"

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
        if (success)
            threadState_->state = Connection::STATE_XMLRPC_REPLY_RECEIVED;
        else
            threadState_->state = Connection::STATE_LOGIN_FAILED;
        
        beginLogin_ = false;
    }
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

bool XMLRPCLoginThread::PerformXMLRPCLogin()
{
	// create a MD5 hash for the password, MAC address and HDD serial number.
	std::string mac_addr = GetMACaddressString();
	std::string id0 = GetId0String();

	md5wrapper md5;
	std::string password_hash = "$1$" + md5.getHashFromString(password_);
	std::string mac_hash = md5.getHashFromString(mac_addr);
	std::string id0_hash = md5.getHashFromString(id0);
    
    boost::shared_ptr<PocoXMLRPCConnection> rpcConnection;
    try
    {
		if (authentication_ && callMethod_ != std::string("login_to_simulator") )
			rpcConnection = boost::shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(authenticationAddress_.c_str(), boost::lexical_cast<int>(authenticationPort_)));
		else if (callMethod_ == std::string("login_to_simulator"))
			rpcConnection = boost::shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(worldAddress_.c_str(), boost::lexical_cast<int>(worldPort_)));
    } catch(std::exception &e)
    {
        OpenSimProtocolModule::LogError("Could not connect to server. Reason: " + Core::ToString(e.what()) + ".");
        return false;
    }

	boost::shared_ptr<PocoXMLRPCCall> call = rpcConnection->StartXMLRPCCall(callMethod_.c_str());
	if (!call)
        return false;
        
	if (!authentication_ && callMethod_ == std::string("login_to_simulator"))
	{
		call->AddStringMember("first", firstName_.c_str());
		call->AddStringMember("last", lastName_.c_str());
		call->AddStringMember("passwd", password_hash.c_str());
     }
	else if (authentication_ && callMethod_ == std::string("ClientAuthentication"))
	{
		std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" +authenticationPort_; 
		call->AddStringMember("account", account.c_str());
		call->AddStringMember("passwd", password_hash.c_str());
		std::string loginuri = "";
	
		loginuri = loginuri+worldAddress_+":"+ worldPort_;
		call->AddStringMember("loginuri", loginuri.c_str());
	}
	else if (authentication_ && callMethod_ == std::string("login_to_simulator"))
	{

		call->AddStringMember("sessionhash", threadState_->parameters.sessionHash.c_str());
		std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" + authenticationPort_; 
		call->AddStringMember("account", account.c_str());
		std::string address = authenticationAddress_ + ":" + authenticationPort_;
		call->AddStringMember("AuthenticationAddress", address.c_str());
		std::string loginuri = "";
		if (!worldAddress_.find("http") != std::string::npos )
			loginuri = "http://";
		
		loginuri = loginuri + worldAddress_ + ":" + worldPort_;
		call->AddStringMember("loginuri", loginuri.c_str());
	}

	call->AddStringMember("start", "last"); // Starting position perhaps?
    call->AddStringMember("version", "realXtend 1.20.13.91224");  ///\todo Make build system create versioning information.
	call->AddStringMember("channel", "realXtend");
	call->AddStringMember("platform", "Win"); ///\todo.
	call->AddStringMember("mac", mac_hash.c_str());
	call->AddStringMember("id0", id0_hash.c_str());
	call->AddIntMember("last_exec_event", 0); // ?

	// The contents of 'options' array unknown. ///\todo Go through them and identify what they really affect.
	PocoXMLRPCCall::StringArray arr = call->CreateStringArray("options");
	call->AddStringToArray(arr, "inventory-root");
	call->AddStringToArray(arr, "inventory-skeleton");
	call->AddStringToArray(arr, "inventory-lib-root");
	call->AddStringToArray(arr, "inventory-lib-owner");
	call->AddStringToArray(arr, "inventory-skel-lib");
	call->AddStringToArray(arr, "initial-outfit");
	call->AddStringToArray(arr, "gestures");
	call->AddStringToArray(arr, "event_categories");
	call->AddStringToArray(arr, "event_notifications");
	call->AddStringToArray(arr, "classified_categories");
	call->AddStringToArray(arr, "buddy-list");
	call->AddStringToArray(arr, "ui-config");
	call->AddStringToArray(arr, "tutorial_setting");
	call->AddStringToArray(arr, "login-flags");
	call->AddStringToArray(arr, "global-textures");

	if(!rpcConnection->FinishXMLRPCCall(call))
	    return false;

	bool loginresult = false;
	if (!authentication_)
	{
		threadState_->parameters.sessionID.FromString(call->GetReplyString("session_id"));
		threadState_->parameters.agentID.FromString(call->GetReplyString("agent_id"));
		threadState_->parameters.circuitCode = call->GetReplyInt("circuit_code");
		std::cout <<threadState_->parameters.sessionID << std::endl;
		loginresult = true;
	}
	else if (authentication_ && callMethod_ != std::string("login_to_simulator")) 
	{
		// Authentication results 
		threadState_->parameters.sessionHash = call->GetReplyString("sessionHash");
		threadState_->parameters.gridUrl = std::string(call->GetReplyString("gridUrl"));
		threadState_->parameters.avatarStorageUrl = std::string(call->GetReplyString("avatarStorageUrl"));
		loginresult = true;
	}
	else if (authentication_ && callMethod_ == std::string("login_to_simulator"))
	{
		threadState_->parameters.sessionID.FromString(call->GetReplyString("session_id"));
		threadState_->parameters.agentID.FromString(call->GetReplyString("agent_id"));
		threadState_->parameters.circuitCode = call->GetReplyInt("circuit_code");
		loginresult = true;
	}

	XMLRPC_RequestFree(call->reply, 1);

	return loginresult;
}

}
