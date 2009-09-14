// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include "RexProtocolMsgIDs.h"

#include "Poco/Net/NetException.h"

#include "HttpRequest.h"

namespace
{

// Writer callback for cURL.
size_t WriteCallback(char *data, size_t size, size_t nmemb, std::vector<char> *buffer)
{
    if (buffer)
    {
        buffer->insert(buffer->end(), data, data + size * nmemb);
        return size * nmemb;
    }
    else
        return 0;
}

}

namespace OpenSimProtocol
{

OpenSimProtocolModule::OpenSimProtocolModule() :
ModuleInterfaceImpl(Foundation::Module::MT_OpenSimProtocol),
connected_(false)
{
}

// virtual
OpenSimProtocolModule::~OpenSimProtocolModule()
{
}

// virtual
void OpenSimProtocolModule::Load()
{
    LogInfo("System " + Name() + " loaded.");
}

// virtual
void OpenSimProtocolModule::Unload()
{
    LogInfo("System " + Name() + " unloaded.");
}

// virtual
void OpenSimProtocolModule::Initialize()
{
    ///\todo Read the template filename from a config file.
    const char *filename = "./data/message_template.msg";

    networkManager_ = boost::shared_ptr<NetMessageManager>(new NetMessageManager(filename));
    assert(networkManager_);
        
    networkManager_->RegisterNetworkListener(this);
    
    // Register event categories.
    eventManager_ = framework_->GetEventManager();
    networkStateEventCategory_ = eventManager_->RegisterEventCategory("NetworkState");
    networkEventInCategory_ = eventManager_->RegisterEventCategory("OpenSimNetworkIn");
    networkEventOutCategory_ = eventManager_->RegisterEventCategory("OpenSimNetworkOut");

    LogInfo("System " + Name() + " initialized.");
}

// virtual 
void OpenSimProtocolModule::Uninitialize()
{
    if(connected_)
        DisconnectFromRexServer();
    
    networkManager_->UnregisterNetworkListener(this);
  
    LogInfo("System " + Name() + " uninitialized.");
}

// virtual
void OpenSimProtocolModule::Update(Core::f64 frametime)
{
    {
        PROFILE(OpenSimProtocolModule_Update);
        if (loginWorker_.IsReady() && loginWorker_.GetState() == Connection::STATE_XMLRPC_REPLY_RECEIVED)
        {
            // XML-RPC reply received; get the login parameters and signal that we're ready to
            // establish an UDP connection.
            clientParameters_ = loginWorker_.GetClientParameters();
            loginWorker_.SetConnectionState(Connection::STATE_INIT_UDP);
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
                LogInfo("Network error occured. Closing server connection.");
                DisconnectFromRexServer();
            }
        }
    }
    RESETPROFILER;
}

//virtual
void OpenSimProtocolModule::OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg)
{
    // Send a Network event. The message ID functions as the event ID.
    NetworkEventInboundData data(msgID, msg);
    eventManager_->SendEvent(networkEventInCategory_, msgID, &data);
}
    
//virtual
void OpenSimProtocolModule::OnNetworkMessageSent(const NetOutMessage *msg)
{
    // Send a NetworkOutStats event.
    NetworkEventOutboundData data(msg->GetMessageID(), msg);
    eventManager_->SendEvent(networkEventOutCategory_, msg->GetMessageID(), &data);
}

void OpenSimProtocolModule::LoginToServer(
    const std::string& first_name,
    const std::string& last_name,
    const std::string& password,
    const std::string& address,
    int port,
    ConnectionThreadState *thread_state)
{   
    std::string callMethod = "login_to_simulator";
    loginWorker_.SetupXMLRPCLogin(first_name, last_name, password, address,
        boost::lexical_cast<std::string>(port), callMethod, thread_state);

    // Start the thread.
    boost::thread(boost::ref(loginWorker_));
}

void OpenSimProtocolModule::LoginToCBServer(
    const std::string& first_name,
    const std::string& last_name,
    const std::string& address,
    int port,
    ConnectionThreadState *thread_state)
{
    std::string callMethod = "login_to_simulator";
    loginWorker_.SetupXMLRPCLogin(first_name, last_name, "auth_done", address,
        boost::lexical_cast<std::string>(port), callMethod, thread_state, "openid", "openid", "openid", true); 
    
    // Start the thread.
    boost::thread(boost::ref(loginWorker_));
}


bool OpenSimProtocolModule::LoginUsingRexAuthentication(
    const std::string& first_name,
    const std::string& last_name,
    const std::string& password,
    const std::string& address,
    int port,
    const std::string& auth_server_address, 
    const std::string& auth_login,
    ConnectionThreadState *thread_state)
{
    bool authentication = true;
    std::string callMethod = "ClientAuthentication";
    int pos = auth_server_address.rfind(":");
    std::string auth_port = "";
    std::string auth_address = "";
    
    if (pos != std::string::npos)
    {
        auth_port = auth_server_address.substr(pos+1);
        auth_address = auth_server_address.substr(0,pos);
    }
    else
    {
        OpenSimProtocolModule::LogInfo("No port defined for the authentication server, using default port (10001).");    
        auth_port = "10001";
        auth_address = auth_server_address;
    }
    
    loginWorker_.SetupXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod,
        thread_state, auth_login, auth_address, auth_port, authentication);
    
    // Start the thread.
    boost::thread(boost::ref(loginWorker_));
    
    return true;
}

bool OpenSimProtocolModule::CreateUDPConnection(const char *address, int port)
{
    loginWorker_.SetConnectionState(Connection::STATE_INIT_UDP);

    bool udp_success = networkManager_->ConnectTo(address, port);
    if (!udp_success)
        return false;

    loginWorker_.SetConnectionState(Connection::STATE_CONNECTED);

    // Send event indicating a succesfull connection.
    eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_SERVER_CONNECTED, NULL);
    connected_ = true;

    // Request capabilities from the server.
//    RequestCapabilities(GetClientParameters().seedCapabilities);
    Core::Thread thread(boost::bind(&OpenSimProtocolModule::RequestCapabilities, this,
        GetClientParameters().seedCapabilities));
    return true;
}

void OpenSimProtocolModule::DisconnectFromRexServer()
{
    if(!connected_)
        return;

    networkManager_->Disconnect();
    loginWorker_.SetConnectionState(Connection::STATE_DISCONNECTED);
    connected_ = false;
    capabilities_.clear();

    eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_SERVER_DISCONNECTED, NULL);
}

void OpenSimProtocolModule::DumpNetworkMessage(NetMsgID id, NetInMessage *msg)
{
    networkManager_->DumpNetworkMessage(id, msg);
}

NetOutMessage *OpenSimProtocolModule::StartMessageBuilding(NetMsgID msgId)
{
    return networkManager_->StartNewMessage(msgId);
}

void OpenSimProtocolModule::FinishMessageBuilding(NetOutMessage *msg)
{
    networkManager_->FinishMessage(msg);
}

std::string OpenSimProtocolModule::GetCapability(const std::string &name)
{
    caps_map_it_t it = capabilities_.find(name);
    if (it == capabilities_.end())
        return "";
    else
        return it->second;
}

void OpenSimProtocolModule::SetCapability(const std::string &name, const std::string &url)
{
    std::pair<caps_map_it_t, bool> ret;

    ret = capabilities_.insert(std::pair<std::string, std::string>(name, url)); 
    if (ret.second == false)
    {
        LogError("Capability " + name + "already exists with an URL " + ret.first->second);
    }
}

void OpenSimProtocolModule::RequestCapabilities(const std::string &seed)
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

    std::vector<Core::u8> response = request.GetResponseData();
    if (response.size() == 0)
    {
        LogError("Size of the response data to \"SEED\" capabilities message was zero.");
        return;
    }

    response.push_back('\0');
    std::string response_str = (char *)&response[0];
    
    ExtractCapabilitiesFromXml(response_str);
}

void OpenSimProtocolModule::ExtractCapabilitiesFromXml(std::string xml)
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

        std::cout << name << std::endl;

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
   POCO_EXPORT_CLASS(OpenSimProtocolModule)
POCO_END_MANIFEST
