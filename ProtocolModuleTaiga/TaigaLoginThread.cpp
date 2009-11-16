// For conditions of distribution and use, see copyright notice in license.txt

/// @file TaigaLoginThread.cpp
/// @brief XML-RPC login worker.

#include "StableHeaders.h"
#include "TaigaLoginThread.h"
#include "ProtocolModuleTaiga.h"
#include "XmlRpcEpi.h"

// ProtocolUtilities includes
#include "OpenSim/OpenSimAuth.h"
#include "OpenSim/Grid.h"
#include "OpenSim/BuddyListParser.h"
#include "Inventory/InventoryParser.h"

// Extenal lib includes
#include <boost/shared_ptr.hpp>
#include <utility>
#include <algorithm>
#include "Poco/MD5Engine.h"

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
            ProtocolModuleTaiga::LogError(ex.what());
            return false;
        }

        try
        {
            // CREATE MD5 HASHES
            Poco::MD5Engine md5_engine;
            // for MAC
	        std::string mac_addr = ProtocolUtilities::GetMACaddressString();
            md5_engine.update(mac_addr.c_str(), mac_addr.size());
            std::string mac_hash = md5_engine.digestToHex(md5_engine.digest());
            // for ID0
	        std::string id0 = ProtocolUtilities::GetId0String();
            md5_engine.update(id0.c_str(), id0.size());
            std::string id0_hash = md5_engine.digestToHex(md5_engine.digest());

            call.AddMember("loginuri", worldAddress_.c_str());
            call.AddMember("start", QString("last").toStdString());
            // TODO: Get version from config manager
            call.AddMember("version", QString("realXtend Naali 0.0.2").toStdString());
            call.AddMember("channel", QString("realXtend").toStdString());
            // TODO: Get platform from OS
            call.AddMember("platform", QString("Win").toStdString());
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
            threadState_->parameters.gridUrl = ProtocolUtilities::GridParser::ExtractGridAddressFromXMLRPCReply(call);

            if (threadState_->parameters.gridUrl.size() == 0)
                throw XmlRpcException("Failed to extract sim_ip and sim_port from login_to_simulator reply!");
            if (threadState_->parameters.sessionID.ToString() == std::string("") ||
                threadState_->parameters.agentID.ToString() == std::string("") ||
                threadState_->parameters.circuitCode == 0)
                throw XmlRpcException("Failed to receive sessionID, agentID or circuitCode from login_to_simulator reply!");

            // Inventory
            try
            {
                threadState_->parameters.inventory = ProtocolUtilities::InventoryParser::ExtractInventoryFromXMLRPCReply(call);
            }
            catch (XmlRpcException &e)
            {
                ProtocolModuleTaiga::LogWarning(QString("Failed to read inventory: %1").arg(e.what()).toStdString());
                threadState_->parameters.inventory = boost::shared_ptr<ProtocolUtilities::InventorySkeleton>(new ProtocolUtilities::InventorySkeleton);
                ProtocolUtilities::InventoryParser::SetErrorFolder(threadState_->parameters.inventory->GetRoot());
            }

            // Buddy List
            try
            {
                threadState_->parameters.buddy_list = ProtocolUtilities::BuddyListParser::ExtractBuddyListFromXMLRPCReply(call);
            }
            catch (XmlRpcException &e)
            {
                ProtocolModuleTaiga::LogWarning(QString("Failed to read buddy list: %1").arg(e.what()).toStdString());
                threadState_->parameters.buddy_list = ProtocolUtilities::BuddyListPtr(new ProtocolUtilities::BuddyList());
            }
        }
        catch(XmlRpcException& ex)
        {
            ProtocolModuleTaiga::LogError(QString("Login procedure threw a XMLRPCException \nReason: %1").arg(ex.what()).toStdString());
            try
            {
                // TODO: transfer error message to login screen
                threadState_->errorMessage = call.GetReply<std::string>("message");
                ProtocolModuleTaiga::LogError(QString("\nMessage: %1").arg(QString(threadState_->errorMessage.c_str())).toStdString());
            }
            catch (XmlRpcException &ex)
            {
                ProtocolModuleTaiga::LogError(QString("\nMessage: <No Message Recieved>").toStdString());
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

}
