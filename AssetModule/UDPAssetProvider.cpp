// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEvents.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "ProtocolModuleOpenSim.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "UDPAssetProvider.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"
#include "NetworkMessages/NetInMessage.h"
#include "NetworkMessages/NetOutMessage.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    const float UDPAssetProvider::DEFAULT_ASSET_TIMEOUT = 120.0;

    UDPAssetProvider::UDPAssetProvider(Foundation::Framework* framework) :
        framework_(framework)
    {
        asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "udp_timeout", DEFAULT_ASSET_TIMEOUT);

        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        event_category_ = event_manager->QueryEventCategory("Asset");
    }

    UDPAssetProvider::~UDPAssetProvider()
    {
    }

    const std::string& UDPAssetProvider::Name()
    {
        static const std::string name("Legacy UDP");
        return name;
    }

    bool UDPAssetProvider::IsValidId(const std::string& asset_id, const std::string& asset_type)
    {
        return RexUUID::IsValid(asset_id);
    }

    bool UDPAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
    {
        if (!IsValidId(asset_id, asset_type))
            return false;

        RexUUID uuid(asset_id);
        if (uuid.IsNull())
            return false;

        // The UDP asset provider only understands the fixed set of asset types used in OpenSim. If the string type is something
        // else, we can't satisfy the request.
        asset_type_t asset_type_int = GetAssetTypeFromTypeName(asset_type);
        if (asset_type_int < 0)
            return false;

        AssetRequest new_request;
        new_request.asset_id_ = asset_id;
        new_request.asset_type_ = asset_type_int;
        new_request.tags_.push_back(tag);
        pending_requests_.push_back(new_request);

        return true;
    }

    bool UDPAssetProvider::InProgress(const std::string& asset_id)
    {
        UDPAssetTransfer* transfer = GetTransfer(asset_id);
        return (transfer != 0);
    }

    Foundation::AssetPtr UDPAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)
    {
        UDPAssetTransfer* transfer = GetTransfer(asset_id);

        if ((transfer) && (transfer->GetReceivedContinuous() >= received))
        {
            // Make new temporary asset for the incomplete data
            RexAsset* new_asset = new RexAsset(transfer->GetAssetId(), GetTypeNameFromAssetType(transfer->GetAssetType()));
            Foundation::AssetPtr asset_ptr(new_asset);
            
            RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
            data.resize(transfer->GetReceivedContinuous());
            transfer->AssembleData(&data[0]);

            return asset_ptr;
        }

        return Foundation::AssetPtr();
    }

    void UDPAssetProvider::SetCurrentProtocolModule(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule)
    {
        protocolModule_ = protocolModule;
        if (protocolModule_.lock().get())
            AssetModule::LogDebug("Current ProtocolModule set succesfully");
        else
            AssetModule::LogWarning("Could not acquire handle to current protocol module");
    }

    bool UDPAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)    
    {
        UDPAssetTransfer* transfer = GetTransfer(asset_id);

        if (transfer)
        {
            size = transfer->GetSize();
            received = transfer->GetReceived();
            received_continuous = transfer->GetReceivedContinuous();

            return true;
        }

        return false;
    }

    void UDPAssetProvider::Update(f64 frametime)
    {
        // Get network interface
        boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net = protocolModule_.lock();

        if ((!net) || (!net->IsConnected()))
        {
            // Connection lost, make any current transfers pending & do nothing until connection returns    
            MakeTransfersPending();
            return;
        }

        // Connection exists, send any pending requests
        SendPendingRequests(net);

        // Handle timeouts for texture & asset transfers
        // Disable asset timeouts for now, a long transfer may stall all others on the server
        // HandleTextureTimeouts(net, frametime);
        // HandleAssetTimeouts(net, frametime);
    }

    void UDPAssetProvider::MakeTransfersPending()
    {
        UDPAssetTransferMap::iterator i = texture_transfers_.begin();
        while (i != texture_transfers_.end())
        {
            AssetRequest new_request;
            new_request.asset_id_ = i->second.GetAssetId();
            new_request.asset_type_ = i->second.GetAssetType();
            new_request.tags_ = i->second.GetTags();
            pending_requests_.push_back(new_request);
            ++i;
        }

        UDPAssetTransferMap::iterator j = asset_transfers_.begin();
        while (j != asset_transfers_.end())
        {
            AssetRequest new_request;
            new_request.asset_id_ = j->second.GetAssetId();
            new_request.asset_type_ = j->second.GetAssetType();
            new_request.tags_ = j->second.GetTags();
            pending_requests_.push_back(new_request);
            ++j;
        }

        texture_transfers_.clear();
        asset_transfers_.clear();
    }

    void UDPAssetProvider::HandleTextureTimeouts(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net, f64 frametime)
    {
        UDPAssetTransferMap::iterator i = texture_transfers_.begin();
        std::vector<RexUUID> erase_tex;
        while(i != texture_transfers_.end())
        {
            UDPAssetTransfer& transfer = i->second;
            if (!transfer.Ready())
            {
                transfer.AddTime(frametime);
                if (transfer.GetTime() > asset_timeout_)
                {
                    RexUUID asset_uuid(transfer.GetAssetId());

                    AssetModule::LogInfo("Texture transfer " + transfer.GetAssetId() + " timed out.");

                    // Send cancel message
                    const ProtocolUtilities::ClientParameters& client = net->GetClientParameters();
                    ProtocolUtilities::NetOutMessage *m = net->StartMessageBuilding(RexNetMsgRequestImage);
                    assert(m);
                    
                    m->AddUUID(client.agentID);
                    m->AddUUID(client.sessionID);

                    m->SetVariableBlockCount(1);
                    m->AddUUID(asset_uuid); // Image UUID
                    m->AddS8(-1); // Discard level, -1 = cancel
                    m->AddF32(0.0); // Download priority, 0 = cancel
                    m->AddU32(0); // Starting packet
                    m->AddU8(RexIT_Normal); // Image type
                    m->MarkReliable();
                    net->FinishMessageBuilding(m);

                    // Send transfer canceled event
                    SendAssetCanceled(transfer);

                    erase_tex.push_back(i->first);
                }
            }

            ++i;
        }

        for(int j = 0; j < erase_tex.size(); ++j)
            texture_transfers_.erase(erase_tex[j]);
    }

    void UDPAssetProvider::HandleAssetTimeouts(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net, f64 frametime)
    {
        UDPAssetTransferMap::iterator i = asset_transfers_.begin();
        std::vector<RexUUID> erase_asset;
        while(i != asset_transfers_.end())
        {
            bool erased = false;

            UDPAssetTransfer& transfer = i->second;
            if (!transfer.Ready())
            {
                transfer.AddTime(frametime);
                if (transfer.GetTime() > asset_timeout_)
                {
                    AssetModule::LogInfo("Asset transfer " + transfer.GetAssetId() + " timed out.");

                    // Send cancel message
                    ProtocolUtilities::NetOutMessage *m = net->StartMessageBuilding(RexNetMsgTransferAbort);
                    assert(m);
                    m->AddUUID(i->first); // Transfer ID
                    m->AddS32(RexAC_Asset); // Asset channel type
                    m->MarkReliable();
                    net->FinishMessageBuilding(m);

                    // Send transfer canceled event
                    SendAssetCanceled(transfer);

                    erase_asset.push_back(i->first);
                }
            }

            ++i;
        }

        for (int j = 0; j < erase_asset.size(); ++j)
            asset_transfers_.erase(erase_asset[j]);
    }

    void UDPAssetProvider::SendPendingRequests(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net)
    {
        AssetRequestVector::iterator i = pending_requests_.begin();
        while(i != pending_requests_.end())
        {
            RexUUID asset_uuid(i->asset_id_);
            if (i->asset_type_ == RexAT_Texture)
                RequestTexture(net, asset_uuid, i->tags_);
            else
                RequestOtherAsset(net, asset_uuid, i->asset_type_, i->tags_);

            i = pending_requests_.erase(i);
        }
    }

    void UDPAssetProvider::RequestTexture(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net, 
        const RexUUID& asset_id, const RequestTagVector& tags)
    {
        // If request already exists, just append the new tag(s)
        std::string asset_id_str = asset_id.ToString();
        UDPAssetTransfer* transfer = GetTransfer(asset_id_str);
        if (transfer)
        {
            transfer->InsertTags(tags);
            return;
        }

        const ProtocolUtilities::ClientParameters& client = net->GetClientParameters();

        UDPAssetTransfer new_transfer;
        new_transfer.SetAssetId(asset_id.ToString());
        new_transfer.SetAssetType(RexAT_Texture);
        new_transfer.InsertTags(tags);
        texture_transfers_[asset_id] = new_transfer;

        AssetModule::LogDebug("Requesting texture " + asset_id.ToString());

        ProtocolUtilities::NetOutMessage *m = net->StartMessageBuilding(RexNetMsgRequestImage);
        assert(m);

        m->AddUUID(client.agentID);
        m->AddUUID(client.sessionID);

        m->SetVariableBlockCount(1);
        m->AddUUID(asset_id); // Image UUID
        m->AddS8(0); // Discard level
        m->AddF32(100.0); // Download priority
        m->AddU32(0); // Starting packet
        m->AddU8(RexIT_Normal); // Image type
        m->MarkReliable();
        net->FinishMessageBuilding(m);
    }

    void UDPAssetProvider::RequestOtherAsset(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net,
        const RexUUID& asset_id, uint asset_type, const RequestTagVector& tags)
    {
        // If request already exists, just append the new tag(s)
        std::string asset_id_str = asset_id.ToString();
        UDPAssetTransfer* transfer = GetTransfer(asset_id_str);
        if (transfer)
        {
            transfer->InsertTags(tags);
            return;
        }

        RexUUID transfer_id;
        transfer_id.Random();

        UDPAssetTransfer new_transfer;
        new_transfer.SetAssetId(asset_id_str);
        new_transfer.SetAssetType(asset_type);
        new_transfer.InsertTags(tags);
        asset_transfers_[transfer_id] = new_transfer;

        AssetModule::LogDebug("Requesting asset " + asset_id_str);

        ProtocolUtilities::NetOutMessage *m = net->StartMessageBuilding(RexNetMsgTransferRequest);
        assert(m);

        m->AddUUID(transfer_id); // Transfer ID
        m->AddS32(RexAC_Asset); // Asset channel type
        m->AddS32(RexAS_Asset); // Asset source type
        m->AddF32(100.0); // Download priority
        
        u8 asset_info[20]; // Asset info block with UUID and type
        memcpy(&asset_info[0], &asset_id.data, 16);
        memcpy(&asset_info[16], &asset_type, 4);
        m->AddBuffer(20, asset_info);
        m->MarkReliable();
        net->FinishMessageBuilding(m);

        return;
    }

    bool UDPAssetProvider::HandleNetworkEvent(IEventData* data)
    {
        ProtocolUtilities::NetworkEventInboundData *event_data = dynamic_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
        if (!event_data)
            return false;

        const ProtocolUtilities::NetMsgID msgID = event_data->messageID;
        ProtocolUtilities::NetInMessage *msg = event_data->message;

        switch(msgID)
        {
        case RexNetMsgImageData:
            HandleTextureHeader(msg);
            return true;

        case RexNetMsgImagePacket:
            HandleTextureData(msg);
            return true;

        case RexNetMsgImageNotInDatabase:
            HandleTextureCancel(msg);
            return true;

        case RexNetMsgTransferInfo:
            HandleAssetHeader(msg);
            return true;

        case RexNetMsgTransferPacket:
            HandleAssetData(msg);
            return true;

        case RexNetMsgTransferAbort:
            HandleAssetCancel(msg);
            return true;

        default:
            return false;
        }
    }

    void UDPAssetProvider::ClearAllTransfers()
    {
        pending_requests_.clear();
        asset_transfers_.clear();
        texture_transfers_.clear();
    }

    void UDPAssetProvider::HandleTextureHeader(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogDebug("Data received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }

        UDPAssetTransfer& transfer = i->second;

        u8 codec = msg->ReadU8();
        u32 size = msg->ReadU32();
        u16 packets = msg->ReadU16();

        transfer.SetSize(size);

        //uint data_size; 
        size_t data_size;
        const u8* data = msg->ReadBuffer(&data_size); // ImageData block
        transfer.ReceiveData(0, data, data_size);

        SendAssetProgress(transfer);

        if (transfer.Ready())
        {
            StoreAsset(transfer);
            texture_transfers_.erase(i);
        }
    }

    void UDPAssetProvider::HandleTextureData(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogDebug("Data received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }

        UDPAssetTransfer& transfer = i->second;

        u16 packet_index = msg->ReadU16();

        //uint data_size; 
        size_t data_size;
        const u8* data = msg->ReadBuffer(&data_size); // ImageData block
        transfer.ReceiveData(packet_index, data, data_size);

        SendAssetProgress(transfer);

        if (transfer.Ready())
        {
            StoreAsset(transfer);
            texture_transfers_.erase(i);
        }
    }

    void UDPAssetProvider::HandleTextureCancel(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogDebug("Cancel received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }

        UDPAssetTransfer& transfer = i->second;

        // Send transfer canceled event
        SendAssetCanceled(transfer);

        AssetModule::LogDebug("Transfer of texture " + asset_id.ToString() + " canceled");
        texture_transfers_.erase(i);
    }

    void UDPAssetProvider::HandleAssetHeader(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogDebug("Data received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }

        UDPAssetTransfer& transfer = i->second;

        s32 channel_type = msg->ReadS32();
        s32 target_type = msg->ReadS32();
        s32 status = msg->ReadS32();
        s32 size = msg->ReadS32();

        if ((status != RexTS_Ok) && (status != RexTS_Done))
        {
            AssetModule::LogDebug("Transfer for asset " + transfer.GetAssetId() + " canceled with code " + ToString<s32>(status));
            asset_transfers_.erase(i);
            return;
        }

        transfer.SetSize(size);
        SendAssetProgress(transfer);

        // We may get data packets before header, so check if all already received
        if (transfer.Ready())
        {
            StoreAsset(transfer);
            asset_transfers_.erase(i);
        }
    }

    void UDPAssetProvider::HandleAssetData(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogDebug("Data received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }

        UDPAssetTransfer& transfer = i->second;

        s32 channel_type = msg->ReadS32();
        s32 packet_index = msg->ReadS32();
        s32 status = msg->ReadS32();

        if ((status != RexTS_Ok) && (status != RexTS_Done))
        {
            AssetModule::LogDebug("Transfer for asset " + transfer.GetAssetId() + " canceled with code " + ToString<s32>(status));

            // Send transfer canceled event
            SendAssetCanceled(transfer);
            
            asset_transfers_.erase(i);
            return;
        }

        //uint data_size; 
        size_t data_size;
        const u8* data = msg->ReadBuffer(&data_size); // Data block
        transfer.ReceiveData(packet_index, data, data_size);

        SendAssetProgress(transfer);

        if (transfer.Ready())
        {
            StoreAsset(transfer);
            asset_transfers_.erase(i);
        }
    }

    void UDPAssetProvider::HandleAssetCancel(ProtocolUtilities::NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        UDPAssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogDebug("Cancel received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }
        
        UDPAssetTransfer& transfer = i->second;

        // Send transfer canceled event
        SendAssetCanceled(transfer);

        AssetModule::LogDebug("Transfer for asset " + transfer.GetAssetId() + " canceled");
        asset_transfers_.erase(i);
    }

    void UDPAssetProvider::SendAssetProgress(UDPAssetTransfer& transfer)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetProgress event_data(transfer.GetAssetId(), GetTypeNameFromAssetType(transfer.GetAssetType()),
            transfer.GetSize(), transfer.GetReceived(), transfer.GetReceivedContinuous());
        event_manager->SendEvent(event_category_, Events::ASSET_PROGRESS, &event_data);
    }

    void UDPAssetProvider::SendAssetCanceled(UDPAssetTransfer& transfer)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetCanceled event_data(transfer.GetAssetId(), GetTypeNameFromAssetType(transfer.GetAssetType()));
        event_manager->SendEvent(event_category_, Events::ASSET_CANCELED, &event_data);
    }

    UDPAssetTransfer* UDPAssetProvider::GetTransfer(const std::string& asset_id)
    {
        RexUUID asset_uuid(asset_id);

        UDPAssetTransferMap::iterator i = texture_transfers_.find(asset_uuid);
        if (i != texture_transfers_.end())
            return &i->second;

        UDPAssetTransferMap::iterator j = asset_transfers_.begin();
        while (j != asset_transfers_.end())
        {
            if (j->second.GetAssetId() == asset_id)
                return &j->second;
            ++j;
        }

        return 0;
    }

    void UDPAssetProvider::StoreAsset(UDPAssetTransfer& transfer)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
            service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
        {
            const std::string& asset_id = transfer.GetAssetId();

            Foundation::AssetPtr new_asset = Foundation::AssetPtr(new RexAsset(asset_id, GetTypeNameFromAssetType(transfer.GetAssetType())));
            RexAsset::AssetDataVector& data = checked_static_cast<RexAsset*>(new_asset.get())->GetDataInternal();
            data.resize(transfer.GetReceived());
            transfer.AssembleData(&data[0]);

            asset_service->StoreAsset(new_asset);

            // Send asset ready event for each request tag
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            const RequestTagVector& tags = transfer.GetTags();
            for(uint i = 0; i < tags.size(); ++i)
            {
                Events::AssetReady event_data(new_asset->GetId(), new_asset->GetType(), new_asset, tags[i]);
                event_manager->SendEvent(event_category_, Events::ASSET_READY, &event_data);
            }
        }
        else
        {
            AssetModule::LogError("Asset service not found, could not store asset to cache");
        }
    }
    
    Foundation::AssetTransferInfoVector UDPAssetProvider::GetTransferInfo()
    {
        Foundation::AssetTransferInfoVector ret;
        
        UDPAssetTransferMap::iterator i = texture_transfers_.begin();
        while (i != texture_transfers_.end())
        {
            Foundation::AssetTransferInfo info;
            info.id_ = i->second.GetAssetId();
            info.type_ = GetTypeNameFromAssetType(i->second.GetAssetType());
            info.provider_ = Name();
            info.size_ = i->second.GetSize();
            info.received_ = i->second.GetReceived();
            info.received_continuous_ = i->second.GetReceivedContinuous();
            ret.push_back(info);
            ++i;
        }
        
        i = asset_transfers_.begin();
        while (i != asset_transfers_.end())
        {
            Foundation::AssetTransferInfo info;
            info.id_ = i->second.GetAssetId();
            info.type_ = GetTypeNameFromAssetType(i->second.GetAssetType());
            info.provider_ = Name();
            info.size_ = i->second.GetSize();
            info.received_ = i->second.GetReceived();
            info.received_continuous_ = i->second.GetReceivedContinuous();
            ret.push_back(info);        
            ++i;
        }
        
        return ret;
    }
    
}
