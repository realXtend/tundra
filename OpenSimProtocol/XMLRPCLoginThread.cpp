// For conditions of distribution and use, see copyright notice in license.txt

/// @file XMLRPCLoginThread.cpp
/// @brief

#include <boost/shared_ptr.hpp>

#include "StableHeaders.h"

#include "XMLRPCLoginThread.h"
#include "OpenSimProtocolModule.h"
#include "XMLRPCEPI.h"
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
    
    XMLRPCEPI* call = 0;
    if ( authentication_ && callMethod_ == "ClientAuthentication" ) 
        call = new XMLRPCEPI(callMethod_,authenticationAddress_, authenticationPort_);
    else
    {
        call = new XMLRPCEPI;
        call->Connect(worldAddress_, worldPort_);
        call->CreateCall(callMethod_);
    }

    
        
	if (!authentication_ && callMethod_ == std::string("login_to_simulator"))
	{
		call->AddMember("first", firstName_);
		call->AddMember("last", lastName_);
		call->AddMember("passwd", password_hash);
     }
	else if (authentication_ && callMethod_ == std::string("ClientAuthentication"))
	{
		std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" +authenticationPort_; 
		call->AddMember("account", account);
		call->AddMember("passwd", password_hash);
		std::string loginuri = "";
	
		loginuri = loginuri+worldAddress_+":"+ worldPort_;
		call->AddMember("loginuri", loginuri);
	}
	else if (authentication_ && callMethod_ == std::string("login_to_simulator"))
	{

		call->AddMember("sessionhash", threadState_->parameters.sessionHash);
		std::string account = authenticationLogin_ + "@" + authenticationAddress_ + ":" + authenticationPort_; 
		call->AddMember("account", account);
		std::string address = authenticationAddress_ + ":" + authenticationPort_;
		call->AddMember("AuthenticationAddress", address);
		std::string loginuri = "";
		if (!worldAddress_.find("http") != std::string::npos )
			loginuri = "http://";
		
		loginuri = loginuri + worldAddress_ + ":" + worldPort_;
		call->AddMember("loginuri", loginuri.c_str());
	}

    

    call->AddMember("start", std::string("last")); // Starting position perhaps?
	call->AddMember("version", std::string("realXtend 1.20.13.91224"));  ///\todo Make build system create versioning information.
	call->AddMember("channel", std::string("realXtend"));
	call->AddMember("platform", std::string("Win")); ///\todo.
	call->AddMember("mac", mac_hash);
	call->AddMember("id0", id0_hash);
	call->AddMember("last_exec_event", int(0)); // ?

	// The contents of 'options' array unknown. ///\todo Go through them and identify what they really affect.
	
    std::string arr = "options";
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

	
    call->Send();

	bool loginresult = false;
	if (!authentication_)
	{
        threadState_->parameters.sessionID.FromString(call->GetReply<std::string>("session_id"));
        threadState_->parameters.agentID.FromString(call->GetReply<std::string>("agent_id"));
		threadState_->parameters.circuitCode = call->GetReply<int>("circuit_code");
        
        if ( threadState_->parameters.sessionID.ToString() == std::string("") || threadState_->parameters.agentID.ToString() == std::string("") 
				|| threadState_->parameters.circuitCode == 0 )
			{
				//todo collect xml failure message.			
				loginresult = false;
			}

		loginresult = true;
	}
	else if (authentication_ && callMethod_ != std::string("login_to_simulator")) 
	{
		// Authentication results
        threadState_->parameters.sessionHash = call->GetReply<std::string>("sessionHash");
        threadState_->parameters.gridUrl = std::string(call->GetReply<std::string>("gridUrl"));
        threadState_->parameters.avatarStorageUrl = std::string(call->GetReply<std::string>("avatarStorageUrl"));
		loginresult = true;
	}
	else if (authentication_ && callMethod_ == std::string("login_to_simulator"))
	{
        std::cout << call->GetReply<std::string>("session_id") << std::endl;
        threadState_->parameters.sessionID.FromString(call->GetReply<std::string>("session_id"));
        threadState_->parameters.agentID.FromString(call->GetReply<std::string>("agent_id"));
		threadState_->parameters.circuitCode = call->GetReply<int>("circuit_code");
		loginresult = true;
	}

	
    delete call;
	return loginresult;
}

}
