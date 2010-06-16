// For conditions of distribution and use, see copyright notice in license.txt

/// @file OpenSimLoginThread.cpp
/// @brief XML-RPC login worker.

#include "StableHeaders.h"
#include "OpenSimLoginThread.h"
#include "ProtocolModuleOpenSim.h"
#include "XmlRpcEpi.h"

// ProtocolUtilities includes
#include "OpenSim/OpenSimAuth.h"
#include "OpenSim/Grid.h"
#include "OpenSim/BuddyListParser.h"
#include "Inventory/InventoryParser.h"
#include "Inventory/InventorySkeleton.h"
#include "Md5.h"
#include "Framework.h"
#include "ConfigurationManager.h"

// Extenal lib includes
#include <boost/shared_ptr.hpp>
#include <utility>
#include <algorithm>

#include "Md5.h"

namespace OpenSimProtocol
{
    std::string OpenSimLoginThread::LOGIN_TO_SIMULATOR = "login_to_simulator";
    std::string OpenSimLoginThread::CLIENT_AUTHENTICATION = "ClientAuthentication";
    std::string OpenSimLoginThread::OPTIONS = "options";
    std::string OpenSimLoginThread::REALXTEND_AUTHENTICATION = "realxtend_authentication";
    std::string OpenSimLoginThread::OPENSIM_AUTHENTICATION = "opensim_authentication";

    OpenSimLoginThread::OpenSimLoginThread() 
        : start_login_(false), ready_(false)
    {
    }

    OpenSimLoginThread::~OpenSimLoginThread()
    {
    }

    void OpenSimLoginThread::operator()()
    {
        if (start_login_)
        {
            threadState_->state = ProtocolUtilities::Connection::STATE_WAITING_FOR_XMLRPC_REPLY;
            if ( PerformXMLRPCLogin() )
            {
                if ( authentication_ == OPENSIM_AUTHENTICATION )
                    threadState_->state = ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED;

                if ( authentication_ == REALXTEND_AUTHENTICATION )
                {
                    threadState_->state = ProtocolUtilities::Connection::STATE_XMLRPC_AUTH_REPLY_RECEIVED;
                    callMethod_ = LOGIN_TO_SIMULATOR;
                    if ( PerformXMLRPCLogin() )
                        threadState_->state = ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED;
                    else
                        threadState_->state = ProtocolUtilities::Connection::STATE_LOGIN_FAILED;
                }
            }
            else
                threadState_->state = ProtocolUtilities::Connection::STATE_LOGIN_FAILED;

            start_login_ = false;
        }
    }

    void OpenSimLoginThread::PrepareOpenSimLogin(
        const QString &first_name,
        const QString &last_name,
        const QString &password,
        const QString &worldAddress,
        const QString &worldPort,
        const QString &startLocation,
        ProtocolUtilities::ConnectionThreadState *thread_state)
    {
        firstName_ = first_name.toStdString();
        lastName_ = last_name.toStdString();
        password_ = password.toStdString();
        worldAddress_ = worldAddress.toStdString();
        worldPort_ = worldPort.toStdString();
        startLocation_ = startLocation.toStdString();

        authentication_ = OPENSIM_AUTHENTICATION;
        callMethod_ = LOGIN_TO_SIMULATOR;
        threadState_ = thread_state;

        ready_ = true;
        threadState_->state = ProtocolUtilities::Connection::STATE_INIT_XMLRPC;
        start_login_ = true;
    }

    void OpenSimLoginThread::PrepareRealXtendLogin(
        const QString& password,
        const QString& worldAddress,
        const QString& worldPort,
        ProtocolUtilities::ConnectionThreadState *thread_state,
        const QString& authentication_login,
        const QString& authentication_address,
        const QString& authentication_port,
        const QString& start_location)
    {
        password_ = password.toStdString();
        worldAddress_ = worldAddress.toStdString();
        worldPort_ = worldPort.toStdString();
        authenticationLogin_ = authentication_login.toStdString();
        authenticationAddress_ = authentication_address.toStdString();
        authenticationPort_ = authentication_port.toStdString();
        startLocation_ = start_location.toStdString();

        authentication_ = REALXTEND_AUTHENTICATION;
        callMethod_ = CLIENT_AUTHENTICATION;
        threadState_ = thread_state;

        ready_ = true;
        threadState_->state = ProtocolUtilities::Connection::STATE_INIT_XMLRPC;
        start_login_ = true;
    }

    bool OpenSimLoginThread::PerformXMLRPCLogin()
    {
        using namespace ProtocolUtilities;

        /////////////////////////////////////
        //           INIT CALL             //
        /////////////////////////////////////

        XmlRpcEpi call;
        try
        {
            if (callMethod_ == CLIENT_AUTHENTICATION )
            {
                call.Connect(authenticationAddress_, authenticationPort_);
                call.CreateCall(callMethod_);
            }
            else if (callMethod_ == LOGIN_TO_SIMULATOR )
            {
                call.Connect(worldAddress_, worldPort_);
                call.CreateCall(callMethod_);
            }
        }
        catch(XmlRpcException& ex)
        {
            threadState_->errorMessage = ex.what();
            ProtocolModuleOpenSim::LogError(ex.what());
            return false;
        }

        try
        {
            // Create MD5 hashes.
            std::string password_hash = "$1$" + GetMd5Hash(password_);
            std::string mac_hash = GetMd5Hash(GetMACaddressString());
            std::string id0_hash = GetMd5Hash(GetId0String());

            // Gather version information.
            const std::string &group = Foundation::Framework::ConfigurationGroup();
            const char *major = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_major").c_str();
            const char *minor = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_minor").c_str();

            // OPENSIM LOGIN, 1st and only iteration
            if ( authentication_ == OPENSIM_AUTHENTICATION && callMethod_ == LOGIN_TO_SIMULATOR )
            {
                call.AddMember("first", firstName_);
                call.AddMember("last", lastName_);
                call.AddMember("passwd", password_hash);
            }
            // REALXTEND LOGIN, 1st iteration
            else if (authentication_ == REALXTEND_AUTHENTICATION && callMethod_ == CLIENT_AUTHENTICATION)
            {
                call.AddMember("account", QString("%1@%2:%3").arg(authenticationLogin_.c_str(), authenticationAddress_.c_str(), authenticationPort_.c_str()).toStdString());
                call.AddMember("passwd", password_hash);
                call.AddMember("loginuri", QString("%1:%2").arg(worldAddress_.c_str(), worldPort_.c_str()).toStdString());
            }
            // REALXTEND LOGIN, 2nd iteration
            else if (authentication_ == REALXTEND_AUTHENTICATION && callMethod_ == LOGIN_TO_SIMULATOR )
            {
                call.AddMember("sessionhash", threadState_->parameters.sessionHash);
                call.AddMember("account", QString("%1@%2:%3").arg(authenticationLogin_.c_str(), authenticationAddress_.c_str(), authenticationPort_.c_str()).toStdString());
                call.AddMember("first", "NotReallyNeeded");
                call.AddMember("last", "NotReallyNeeded");
                call.AddMember("passwd", password_hash);
                call.AddMember("AuthenticationAddress", QString("%1:%2").arg(authenticationAddress_.c_str(), authenticationPort_.c_str()).toStdString());
                call.AddMember("loginuri", QString("%1:%2").arg(worldAddress_.c_str(), worldPort_.c_str()).toStdString());
            }

            if (startLocation_.empty() || startLocation_.size() == 0)
            {
                call.AddMember("start", QString("last").toStdString());
            }
            else if (startLocation_.compare("last") == 0 || startLocation_.compare("home") == 0)
            {
                call.AddMember("start", startLocation_);
            }
            else
            {
                if (startLocation_.find("&") == std::string::npos)                
                {
                    call.AddMember("start", "uri:"+startLocation_+"&amp;128&amp;128&amp;0");
                }
                else
                {                   
                    QString startloc(startLocation_.c_str());
                    startloc.replace("&", "&amp;");
                    call.AddMember("start", "uri:"+startloc.toStdString());
                }
            }            

            call.AddMember("version", QString("realXtend Naali %1.%2").arg(major, minor).toStdString());
            call.AddMember("channel", QString("realXtend").toStdString());
            call.AddMember("platform", GetPlatform());
            call.AddMember("mac", mac_hash);
            call.AddMember("id0", id0_hash);
            call.AddMember("last_exec_event", int(0));

            // TODO: Go through them and identify what they really affect.
            call.AddStringToArray(OPTIONS, "inventory-root");
            call.AddStringToArray(OPTIONS, "inventory-skeleton");
            call.AddStringToArray(OPTIONS, "inventory-lib-root");
            call.AddStringToArray(OPTIONS, "inventory-lib-owner");
            call.AddStringToArray(OPTIONS, "inventory-skel-lib");
            call.AddStringToArray(OPTIONS, "initial-outfit");
            call.AddStringToArray(OPTIONS, "gestures");
            call.AddStringToArray(OPTIONS, "event_categories");
            call.AddStringToArray(OPTIONS, "event_notifications");
            call.AddStringToArray(OPTIONS, "classified_categories");
            call.AddStringToArray(OPTIONS, "buddy-list");
            call.AddStringToArray(OPTIONS, "ui-config");
            call.AddStringToArray(OPTIONS, "tutorial_setting");
            call.AddStringToArray(OPTIONS, "login-flags");
            call.AddStringToArray(OPTIONS, "global-textures");
        }
        catch (XmlRpcException& ex)
        {
            ProtocolModuleOpenSim::LogError(ex.what());
            threadState_->errorMessage = ex.what();
            return false;
        }

        /////////////////////////////////////
        //           SEND CALL             //
        /////////////////////////////////////

        try
        {
            call.Send();
        }
        catch(XmlRpcException& ex)
        {
            if (callMethod_ == CLIENT_AUTHENTICATION )
                threadState_->errorMessage = QString("Authentication failed to %1:%2, please check your Authentication address and port.").arg(authenticationAddress_.c_str(), authenticationPort_.c_str()).toStdString();
            else if (callMethod_ == LOGIN_TO_SIMULATOR )
                threadState_->errorMessage = QString("Login failed to %1:%2, please check your World address and port.").arg(worldAddress_.c_str(), worldPort_.c_str()).toStdString();
            ProtocolModuleOpenSim::LogError(ex.what());
            return false;
        }

        /////////////////////////////////////
        //          PARSE RESULTS          //
        /////////////////////////////////////

        try
        {
            if (authentication_ == OPENSIM_AUTHENTICATION && callMethod_ == LOGIN_TO_SIMULATOR)
            {
                // Grid url, Session ID, Agent ID, Cirtuit Code, Seed Caps
                threadState_->parameters.sessionID.FromString(call.GetReply<std::string>("session_id"));
                threadState_->parameters.agentID.FromString(call.GetReply<std::string>("agent_id"));
                threadState_->parameters.circuitCode = call.GetReply<int>("circuit_code");
                threadState_->parameters.seedCapabilities = call.GetReply<std::string>("seed_capability");
                threadState_->parameters.gridUrl = GridParser::ExtractGridAddressFromXMLRPCReply(call);

                if (call.HasReply("webdav_inventory"))
                    threadState_->parameters.webdavInventoryUrl = call.GetReply<std::string>("webdav_inventory");

                if (call.HasReply("region_x") && call.HasReply("region_y"))
                {
                    threadState_->parameters.regionX = static_cast<uint16_t>((call.GetReply<long>("region_x"))/256);
                    threadState_->parameters.regionY = static_cast<uint16_t>((call.GetReply<long>("region_y"))/256);
                }
                
                if (threadState_->parameters.gridUrl.size() == 0)
                    throw XmlRpcException("Failed to extract sim_ip and sim_port from login_to_simulator reply!");
                if (threadState_->parameters.sessionID.ToString() == std::string("") ||
                    threadState_->parameters.agentID.ToString() == std::string("") ||
                    threadState_->parameters.circuitCode == 0)
                    throw XmlRpcException("Failed to receive sessionID, agentID or circuitCode from login_to_simulator reply!");

                // Inventory
                try
                {
                    threadState_->parameters.inventory = InventoryParser::ExtractInventoryFromXMLRPCReply(call);
                }
                catch (XmlRpcException &e)
                {
                    ProtocolModuleOpenSim::LogWarning(QString("Failed to read inventory: %1").arg(e.what()).toStdString());
                    threadState_->parameters.inventory = boost::shared_ptr<InventorySkeleton>(new InventorySkeleton);
                    InventoryParser::SetErrorFolder(threadState_->parameters.inventory->GetRoot());
                }

                // Buddy List
                try
                {
                    threadState_->parameters.buddy_list = BuddyListParser::ExtractBuddyListFromXMLRPCReply(call);
                }
                catch (XmlRpcException &e)
                {
                    ProtocolModuleOpenSim::LogWarning(QString("Failed to read buddy list: %1").arg(e.what()).toStdString());
                    threadState_->parameters.buddy_list = BuddyListPtr(new BuddyList());
                }
            }
            else if (authentication_ == REALXTEND_AUTHENTICATION && callMethod_ == CLIENT_AUTHENTICATION) 
            {
                threadState_->parameters.sessionHash = "";
                threadState_->parameters.avatarStorageUrl = "";
                threadState_->parameters.sessionHash = call.GetReply<std::string>("sessionHash");
                threadState_->parameters.gridUrl = std::string(call.GetReply<std::string>("gridUrl"));
                //\bug the grid url provided by authentication server points to tcp port, but the grid url is used in the code to connect to udp port
                threadState_->parameters.avatarStorageUrl = std::string(call.GetReply<std::string>("avatarStorageUrl"));
            }
            else if (authentication_ == REALXTEND_AUTHENTICATION && callMethod_ == LOGIN_TO_SIMULATOR)
            {
                // Grid url, Session ID, Agent ID, Cirtuit Code, Seed Caps
                threadState_->parameters.sessionID.FromString(call.GetReply<std::string>("session_id"));
                threadState_->parameters.agentID.FromString(call.GetReply<std::string>("agent_id"));
                threadState_->parameters.circuitCode = call.GetReply<int>("circuit_code");
                threadState_->parameters.seedCapabilities = call.GetReply<std::string>("seed_capability");
                if (call.HasReply("region_x") && call.HasReply("region_y"))
                {
                    threadState_->parameters.regionX = static_cast<uint16_t>((call.GetReply<long>("region_x"))/256);
                    threadState_->parameters.regionY = static_cast<uint16_t>((call.GetReply<long>("region_y"))/256);
                }
                ///\bug related to one 10 lines above. instead of using port defined in authentication server, 
                /// use the one given by simulator.
                /// Does this still apply? -jj. Is this a bug in the rex auth server? If so, flag as a workaround or something similar.
                threadState_->parameters.gridUrl = GridParser::ExtractGridAddressFromXMLRPCReply(call);
                if (threadState_->parameters.gridUrl.size() == 0)
                    throw XmlRpcException("Failed to extract sim_ip and sim_port from login_to_simulator reply!");

                // Inventory
                try
                {
                    threadState_->parameters.inventory = InventoryParser::ExtractInventoryFromXMLRPCReply(call);
                }
                catch(XmlRpcException &e)
                {
                    ProtocolModuleOpenSim::LogWarning(QString("Failed to read inventory: %1").arg(e.what()).toStdString());
                    threadState_->parameters.inventory = boost::shared_ptr<InventorySkeleton>(new InventorySkeleton);
                    InventoryParser::SetErrorFolder(threadState_->parameters.inventory->GetRoot());
                }

                // Buddy List
                try
                {
                    threadState_->parameters.buddy_list = BuddyListParser::ExtractBuddyListFromXMLRPCReply(call);
                }
                catch(XmlRpcException &e)
                {
                    ProtocolModuleOpenSim::LogWarning(QString("Failed to read buddy list: %1").arg(e.what()).toStdString());
                    threadState_->parameters.buddy_list = BuddyListPtr(new BuddyList());
                }
            }
            else
                throw XmlRpcException(QString("Undefined login method %1 at parsing call results in PerformXMLRPCLogin()").arg(callMethod_.c_str()).toStdString());
        }
        catch(XmlRpcException& ex)
        {
            ProtocolModuleOpenSim::LogError(QString("Login procedure threw a XMLRPCException >>> Reason: %1").arg(ex.what()).toStdString());
            try
            {
                threadState_->errorMessage = call.GetReply<std::string>("message");
                ProtocolModuleOpenSim::LogError(QString(">>> Message: %1").arg(QString(threadState_->errorMessage.c_str())).toStdString());
            }
            catch (XmlRpcException &/*ex*/)
            {
                threadState_->errorMessage = std::string("Connecting failed, reason unknown. World address probably not valid.");
                ProtocolModuleOpenSim::LogError(QString(">>> Message: <No Message Recieved>").toStdString());
            }
            return false;
        }
        return true;
    }

    volatile ProtocolUtilities::Connection::State OpenSimLoginThread::GetState() const
    {
        if (!ready_)
            return ProtocolUtilities::Connection::STATE_DISCONNECTED;
        else
            return threadState_->state;
    }

    std::string &OpenSimLoginThread::GetErrorMessage() const
    {
        return threadState_->errorMessage;
    }
    
}
