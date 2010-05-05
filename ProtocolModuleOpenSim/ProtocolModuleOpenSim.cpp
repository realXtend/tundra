// For conditions of distribution and use, see copyright notice in license.txt

//#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ProtocolModuleOpenSim.h"

#include "Framework.h"
#include "EventManager.h"
#include "Profiler.h"
#include "ModuleManager.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "HttpRequest.h"

#include <Poco/Net/NetException.h>
#include <Poco/ClassLibrary.h>

#include <QUrl>
#include <QStringList>

#include "MemoryLeakCheck.h"

namespace OpenSimProtocol
{

    ProtocolModuleOpenSim::ProtocolModuleOpenSim() :
        ModuleInterfaceImpl(Foundation::Module::MT_OpenSimProtocol),
        connected_(false),
        authenticationType_(ProtocolUtilities::AT_Unknown)
    {
    }

    // virtual
    ProtocolModuleOpenSim::~ProtocolModuleOpenSim()
    {
    }

    // virtual
    void ProtocolModuleOpenSim::Initialize()
    {
        loginWorker_.SetFramework(GetFramework());
    }

    // virtual 
    void ProtocolModuleOpenSim::Uninitialize()
    {
        thread_.join();

        if (connected_)
            DisconnectFromServer();

        if (networkManager_)
            networkManager_->UnregisterNetworkListener((ProtocolUtilities::INetMessageListener *)this);

        eventManager_.reset();
    }

    void ProtocolModuleOpenSim::RegisterNetworkEvents()
    {
        const char *filename = "./data/message_template.msg";

        networkManager_ = boost::shared_ptr<ProtocolUtilities::NetMessageManager>(new ProtocolUtilities::NetMessageManager(filename));
        assert(networkManager_);
        networkManager_->RegisterNetworkListener(this);

        // Register event categories.
        eventManager_ = framework_->GetEventManager();
        networkStateEventCategory_ = eventManager_->RegisterEventCategory("NetworkState");
        networkEventInCategory_ = eventManager_->RegisterEventCategory("NetworkIn");
        networkEventOutCategory_ = eventManager_->RegisterEventCategory("NetworkOut");

        // Send event that other modules can query above categories
        boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> modulePointerToThis = 
            framework_->GetModuleManager()->GetModule<ProtocolModuleOpenSim>(Foundation::Module::MT_OpenSimProtocol);
        if (modulePointerToThis.lock().get())
        {
            event_category_id_t framework_category_id = eventManager_->QueryEventCategory("Framework");
            ProtocolUtilities::NetworkingRegisteredEvent event_data(modulePointerToThis);
            eventManager_->SendEvent(framework_category_id, Foundation::NETWORKING_REGISTERED, &event_data);
            LogInfo("Sending Networking Registered event");
        }
        LogInfo("Network events [NetworkState, NetworkIn, NetworkOut] registered");
    }

    void ProtocolModuleOpenSim::UnregisterNetworkEvents()
    {
        if (connected_)
            DisconnectFromServer();

        if (networkManager_)
        {
            networkManager_->UnregisterNetworkListener(this);
            networkManager_.reset();
        }

        LogInfo("Networking unregistered.");
    }

    // virtual
    void ProtocolModuleOpenSim::Update(f64 frametime)
    {
        {
            PROFILE(ProtocolModuleOpenSim_Update);
            if (loginWorker_.IsReady() && loginWorker_.GetState() == ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED)
            {
                // XML-RPC reply received; get the login parameters and signal that we're ready to
                // establish an UDP connection.
                clientParameters_ = loginWorker_.GetClientParameters();
                loginWorker_.SetConnectionState(ProtocolUtilities::Connection::STATE_INIT_UDP);
            }
            else if (loginWorker_.GetState() == ProtocolUtilities::Connection::STATE_LOGIN_FAILED)
            {
                eventManager_->SendEvent(networkStateEventCategory_, ProtocolUtilities::Events::EVENT_CONNECTION_FAILED, 0);
                loginWorker_.SetConnectionState(ProtocolUtilities::Connection::STATE_DISCONNECTED);
            }

            if (connected_)
            {
                try
                {
                    networkManager_->ProcessMessages();
                }
                catch(Poco::Net::NetException &e)
                {
                    LogError(e.what());
                    LogError("Network error occured. Closing server connection.");
                    DisconnectFromServer();
                }
            }
        }
        RESETPROFILER;
    }

    //virtual
    void ProtocolModuleOpenSim::OnNetworkMessageReceived(ProtocolUtilities::NetMsgID msgID, ProtocolUtilities::NetInMessage *msg)
    {
        try
        {
            // Send a Network event. The message ID functions as the event ID.
            ProtocolUtilities::NetworkEventInboundData data(msgID, msg);
            eventManager_->SendEvent(networkEventInCategory_, msgID, &data);
        }
        catch(const Exception &e)
        {
            LogInfo(std::string("Warning: Handling an inbound network message threw an exception: ") + e.what());
        }
        catch(const std::exception &e)
        {
            LogInfo(std::string("Warning: Handling an inbound network message threw an exception: ") + e.what());
        }
        catch(...)
        {
            LogInfo("Warning: Handling an inbound network message threw an unknown exception.");
        }
    }

    //virtual
    void ProtocolModuleOpenSim::OnNetworkMessageSent(const ProtocolUtilities::NetOutMessage *msg)
    {
        // Send a NetworkOutStats event.
        ProtocolUtilities::NetworkEventOutboundData data(msg->GetMessageID(), msg);
        eventManager_->SendEvent(networkEventOutCategory_, msg->GetMessageID(), &data);
    }

    bool ProtocolModuleOpenSim::CreateUdpConnection(const char *address, int port)
    {
        loginWorker_.SetConnectionState(ProtocolUtilities::Connection::STATE_INIT_UDP);

        if (networkManager_->ConnectTo(address, port))
        {
            loginWorker_.SetConnectionState(ProtocolUtilities::Connection::STATE_CONNECTED);
            connected_ = true;

            // Send event indicating a succesfull connection
            ProtocolUtilities::AuthenticationEventData auth_data(authenticationType_, "", loginWorker_.GetClientParameters().gridUrl);
            auth_data.inventorySkeleton = loginWorker_.GetClientParameters().inventory;

            // Fill in webdav information if exists
            if (loginWorker_.GetClientParameters().webdavInventoryUrl != "")
            {
                auth_data.webdav_host = loginWorker_.GetClientParameters().webdavInventoryUrl;
                auth_data.webdav_identity = loginWorker_.GetUsername();
                auth_data.webdav_password = loginWorker_.GetPassword();
                auth_data.type = ProtocolUtilities::AT_Taiga;
            }
            eventManager_->SendEvent(networkStateEventCategory_, ProtocolUtilities::Events::EVENT_SERVER_CONNECTED, &auth_data);

            // Request capabilities from the server.
            Thread thread(boost::bind(&ProtocolModuleOpenSim::RequestCapabilities, this, GetClientParameters().seedCapabilities));
            return true;
        }
        else
        {
            LogError("Network Manager could not establish UDP connection");
            return false;
        }
    }

    void ProtocolModuleOpenSim::DisconnectFromServer()
    {
        if (!connected_)
            return;

        networkManager_->Disconnect();
        networkManager_->UnregisterNetworkListener(this);
        networkManager_.reset();
        loginWorker_.SetConnectionState(ProtocolUtilities::Connection::STATE_DISCONNECTED);
        connected_ = false;
        capabilities_.clear();
        clientParameters_.Reset();

        eventManager_->SendEvent(networkStateEventCategory_, ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED, 0);
    }

    void ProtocolModuleOpenSim::DumpNetworkMessage(ProtocolUtilities::NetMsgID id, ProtocolUtilities::NetInMessage *msg)
    {
        networkManager_->DumpNetworkMessage(id, msg);
    }

    ProtocolUtilities::NetOutMessage *ProtocolModuleOpenSim::StartMessageBuilding(ProtocolUtilities::NetMsgID msgId)
    {
        return networkManager_->StartNewMessage(msgId);
    }

    void ProtocolModuleOpenSim::FinishMessageBuilding(ProtocolUtilities::NetOutMessage *msg)
    {
        networkManager_->FinishMessage(msg);
    }

    std::string ProtocolModuleOpenSim::GetCapability(const std::string &name)
    {
        caps_map_it_t it = capabilities_.find(name);
        if (it == capabilities_.end())
            return "";
        else
            return it->second;
    }

    void ProtocolModuleOpenSim::SetCapability(const std::string &name, const std::string &url)
    {
        std::pair<caps_map_it_t, bool> ret;
        ret = capabilities_.insert(std::pair<std::string, std::string>(name, url)); 
        if (ret.second == false)
            LogInfo("Capability " + name + "already exists with an URL " + ret.first->second);
    }

    void ProtocolModuleOpenSim::RequestCapabilities(const std::string &seed)
    {
        ///\todo Do we want request all of these (mostly) deprecated caps we never use anyway?
        std::string msg = "<llsd><array>"
            "<string>ChatSessionRequest</string>"
            "<string>CopyInventoryFromNotecard</string>"
            "<string>DispatchRegionInfo</string>"
            "<string>EstateChangeInfo</string>"
            "<string>EventQueueGet</string>"
            "<string>FetchInventoryDescendents</string>"
            "<string>GroupProposalBallot</string>"
            "<string>MapLayer</string>"
            "<string>MapLayerGod</string>"
            "<string>NewFileAgentInventory</string>"
            "<string>ParcelPropertiesUpdate</string>"
            "<string>ParcelVoiceInfoRequest</string>"
            "<string>ProvisionVoiceAccountRequest</string>"
            "<string>RemoteParcelRequest</string>"
            "<string>RequestTextureDownload</string>"
            "<string>SearchStatRequest</string>"
            "<string>SearchStatTracking</string>"
            "<string>SendPostcard</string>"
            "<string>SendUserReport</string>"
            "<string>SendUserReportWithScreenshot</string>"
            "<string>ServerReleaseNotes</string>"
            "<string>StartGroupProposal</string>"
            "<string>UpdateGestureAgentInventory</string>"
            "<string>UpdateNotecardAgentInventory</string>"
            "<string>UpdateScriptAgentInventory</string>"
            "<string>UpdateGestureTaskInventory</string>"
            "<string>UpdateNotecardTaskInventory</string>"
            "<string>UpdateScriptTaskInventory</string>"
            "<string>ViewerStartAuction</string>"
            "<string>UntrustedSimulatorMessage</string>"
            "<string>ViewerStats</string>"
            "<string>GetTexture</string>"
            "</array></llsd>";

        HttpUtilities::HttpRequest request;
        request.SetUrl(seed);
        request.SetMethod(HttpUtilities::HttpRequest::Post);
        request.SetRequestData("application/xml", msg.c_str());
        request.Perform();

        if (!request.GetSuccess())
        {
            LogError(request.GetReason());
            return;
        }
        
        std::vector<u8> response = request.GetResponseData();
        if (response.size() == 0)
        {
            LogError("Size of the response data to \"SEED\" capabilities message was zero.");
            return;
        }

        response.push_back('\0');
        std::string response_str = (char *)&response[0];

        ExtractCapabilitiesFromXml(response_str);

        eventManager_->SendDelayedEvent(networkStateEventCategory_, ProtocolUtilities::Events::EVENT_CAPS_FETCHED, Foundation::EventDataPtr(), 0);
    }

    void ProtocolModuleOpenSim::ExtractCapabilitiesFromXml(std::string xml)
    {
        const std::string key = "<key>";
        const std::string key_end = "</key>";
        const std::string str = "<string>";
        const std::string str_end = "</string>";
        size_t key_pos = 0, key_end_pos = 0, str_pos = 0, str_end_pos = 0;
        bool proceed = true;

        while(proceed)
        {
            std::string name, url;

            key_pos = xml.find(key, str_end_pos);
            key_end_pos = xml.find(key_end, key_pos);
            name = xml.substr(key_pos + key.length(), key_end_pos - key_pos - key.length());

            str_pos = xml.find(str, key_end_pos);
            str_end_pos = xml.find(str_end, str_pos);
            url = xml.substr(str_pos + str.length(), str_end_pos - str_pos - str.length());

            SetCapability(name, url);

            if ((xml.find(key, str_end_pos) > xml.length()) ||
                (xml.find(key, str_end_pos) == std::string::npos))
                proceed = false;
        }
    }

} // namespace OpenSimProtocol

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OpenSimProtocol;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(ProtocolModuleOpenSim)
POCO_END_MANIFEST
