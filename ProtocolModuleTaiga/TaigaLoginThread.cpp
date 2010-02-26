// For conditions of distribution and use, see copyright notice in license.txt

/// @file TaigaLoginThread.cpp
/// @brief XML-RPC login worker.

//#include "StableHeaders.h"
#include "TaigaLoginThread.h"
#include "ProtocolModuleTaiga.h"
#include "XmlRpcEpi.h"

// ProtocolUtilities includes
#include "OpenSim/OpenSimAuth.h"
#include "OpenSim/Grid.h"
#include "OpenSim/BuddyListParser.h"
#include "Inventory/InventoryParser.h"
#include "Inventory/InventorySkeleton.h"
#include "Md5.h"
#include "Platform.h"
#include "Framework.h"
#include "ConfigurationManager.h"

// Extenal lib includes
#include <boost/shared_ptr.hpp>
#include <utility>
#include <algorithm>

namespace TaigaProtocol
{
    std::string TaigaLoginThread::LOGIN_TO_SIMULATOR = "login_to_simulator";
    std::string TaigaLoginThread::OPTIONS = "options";

    TaigaLoginThread::TaigaLoginThread() 
        : start_login_(false), ready_(false)
    {
    }

    TaigaLoginThread::~TaigaLoginThread()
    {
    }

    void TaigaLoginThread::operator()()
    {
        if (start_login_)
        {
            threadState_->state = ProtocolUtilities::Connection::STATE_WAITING_FOR_XMLRPC_REPLY;
            if ( PerformXMLRPCLogin() )
                threadState_->state =  ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED;
            else
                threadState_->state = ProtocolUtilities::Connection::STATE_LOGIN_FAILED;
            start_login_ = false;
        }
    }

    void TaigaLoginThread::PrepareTaigaLogin(const QString& worldAddress,
                                             const QString& worldPort,
                                             ProtocolUtilities::ConnectionThreadState *thread_state)
    {
        worldAddress_ = worldAddress.toStdString();
        worldPort_ = worldPort.toStdString();
        threadState_ = thread_state;

        ready_ = true;
        threadState_->state = ProtocolUtilities::Connection::STATE_INIT_XMLRPC;
        start_login_ = true;
    }

    bool TaigaLoginThread::PerformXMLRPCLogin()
    {
        using namespace ProtocolUtilities;

        /////////////////////////////////////
        //           INIT CALL             //
        /////////////////////////////////////

        XmlRpcEpi call;
        try
        {
            call.Connect(worldAddress_, worldPort_);
            call.CreateCall(LOGIN_TO_SIMULATOR);
        }
        catch (XmlRpcException& ex)
        {
            threadState_->errorMessage = ex.what();
            ProtocolModuleTaiga::LogError(ex.what());
            return false;
        }

        try
        {
            // Create MD5 hashes.
            std::string mac_hash = GetMd5Hash(GetMACaddressString());
            std::string id0_hash = GetMd5Hash(GetId0String());

            // Gather version information.
            const std::string &group = Foundation::Framework::ConfigurationGroup();
            const char *major = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_major").c_str();
            const char *minor = framework_->GetDefaultConfig().GetSetting<std::string>(group, "version_minor").c_str();

            call.AddMember("start", QString("last").toStdString());
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
            threadState_->errorMessage = ex.what();
            ProtocolModuleTaiga::LogError(ex.what());
            return false;
        }

        /////////////////////////////////////
        //           SEND CALL             //
        /////////////////////////////////////

        try
        {
            call.Send();
        }
        catch (XmlRpcException& ex)
        {
            threadState_->errorMessage = QString("Login failed to %1, please check your World address and port.").arg(worldAddress_.c_str()).toStdString();
            ProtocolModuleTaiga::LogError(ex.what());
            return false;
        }
    
        /////////////////////////////////////
        //          PARSE RESULTS          //
        /////////////////////////////////////

        try
        {
            // Grid url, Session ID, Agent ID, Cirtuit Code, Seed Caps
            threadState_->parameters.sessionID.FromString(call.GetReply<std::string>("session_id"));
            threadState_->parameters.agentID.FromString(call.GetReply<std::string>("agent_id"));
            threadState_->parameters.circuitCode = call.GetReply<int>("circuit_code");
            threadState_->parameters.seedCapabilities = call.GetReply<std::string>("seed_capability");
            threadState_->parameters.gridUrl = GridParser::ExtractGridAddressFromXMLRPCReply(call);

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
                ProtocolModuleTaiga::LogWarning(QString("Failed to read inventory: %1").arg(e.what()).toStdString());
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
                ProtocolModuleTaiga::LogWarning(QString("Failed to read buddy list: %1").arg(e.what()).toStdString());
                threadState_->parameters.buddy_list = BuddyListPtr(new BuddyList());
            }
        }
        catch(XmlRpcException& ex)
        {
            ProtocolModuleTaiga::LogError(QString("Login procedure threw a XMLRPCException >>> Reason: %1").arg(ex.what()).toStdString());
            try
            {
                threadState_->errorMessage = call.GetReply<std::string>("message");
                ProtocolModuleTaiga::LogError(QString(">>> Message: %1").arg(QString(threadState_->errorMessage.c_str())).toStdString());
            }
            catch (XmlRpcException &/*ex*/)
            {
                threadState_->errorMessage = std::string("Connecting failed, reason unknown. World address probably not valid.");
                ProtocolModuleTaiga::LogError(QString(">>> Message: <No Message Recieved>").toStdString());
            }
            return false;
        }
        return true;
    }

    volatile ProtocolUtilities::Connection::State TaigaLoginThread::GetState() const
    {
        if (!ready_)
            return ProtocolUtilities::Connection::STATE_DISCONNECTED;
        else
            return threadState_->state;
    }

    std::string &TaigaLoginThread::GetErrorMessage() const
    {
        return threadState_->errorMessage;
    }
}
