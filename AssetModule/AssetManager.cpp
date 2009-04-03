#include "StableHeaders.h"
#include "AssetModule.h"
#include "AssetInterface.h"
#include "OpenSimProtocolModule.h"
#include "NetInMessage.h"
#include "RexProtocolMsgIDs.h"
#include "AssetDefines.h"
#include "AssetManager.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    const char *AssetManager::DEFAULT_ASSET_CACHE_PATH = "/assetcache";

    AssetManager::AssetTransfer::AssetTransfer() :
        size_(0),
        received_(0)
    {
    }
    
    AssetManager::AssetTransfer::~AssetTransfer()
    {
    }
    
    bool AssetManager::AssetTransfer::Ready() const
    {
        if (!size_) 
            return false; // No header received, size not known yet
        
        return received_ >= size_;
    }
    
    Core::uint AssetManager::AssetTransfer::GetReceivedContinuous() const
    {
        Core::uint size = 0;
        
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        Core::uint expected_index = 0;
      
        while (i != data_packets_.end())
        {
            if (i->first != expected_index)
                break;
            
            size += i->second.size();
            
            ++expected_index;
            ++i;
        }
        
        return size;
    }
    
    void AssetManager::AssetTransfer::ReceiveData(Core::uint packet_index, const Core::u8* data, Core::uint size)
    {
        if (!size)
        {
            AssetModule::LogWarning("Trying to store zero bytes of data");
            return;
        }
        
        if (!data_packets_[packet_index].size())
        {
            data_packets_[packet_index].resize(size);
            memcpy(&data_packets_[packet_index][0], data, size);
            received_ += size;
        }
        else
        {
            AssetModule::LogWarning("Already received asset data packet index " + Core::ToString<Core::uint>(packet_index));
        }
    }
    
    void AssetManager::AssetTransfer::AssembleData(Core::u8* buffer) const
    {
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        Core::uint expected_index = 0;
      
        while (i != data_packets_.end())
        {
            if (i->first != expected_index)
                break;
            
            memcpy(buffer, &i->second[0], i->second.size());
            buffer += i->second.size();
            
            ++expected_index;
            ++i;
        }
    }
    
    AssetManager::AssetManager(Foundation::Framework* framework, OpenSimProtocolModule* net_interface) : 
        framework_(framework),
        net_interface_(net_interface)
    {
        // Create asset cache directory
        std::string cache_path = framework_->GetPlatform()->GetApplicationDataDirectory();
        cache_path += DEFAULT_ASSET_CACHE_PATH;
        if (boost::filesystem::exists(cache_path) == false)
        {
            boost::filesystem::create_directory(cache_path);
        }
    }
    
    AssetManager::~AssetManager()
    {
    }
    
    Foundation::AssetPtr AssetManager::GetAsset(const std::string& asset_id)
    {
        Foundation::AssetPtr no_asset;
        return no_asset;
    }
    
    void AssetManager::RequestAsset(const RexUUID& asset_id, Core::uint asset_type)
    {
        if (assets_.find(asset_id) != assets_.end())
        {
            AssetModule::LogInfo("Asset " + asset_id.ToString() + " already received");
            return;
        }
        
        if (!net_interface_)
        {
            AssetModule::LogError("No netinterface, cannot request assets");
            return;
        }
        
        if (asset_type == RexAT_Texture)
        {
            RequestTexture(asset_id);
        }
        else
        {
            RequestOtherAsset(asset_id, asset_type);
        }
    }
    
    void AssetManager::RequestTexture(const RexUUID& asset_id)
    {
        if (!net_interface_)
            return;
        
        const ClientParameters& client = net_interface_->GetClientParameters();
        
        if (texture_transfers_.find(asset_id) != texture_transfers_.end())
        {
            AssetModule::LogWarning("Texture " + asset_id.ToString() + " already requested");
            return;
        }
        
        AssetTransfer new_transfer;
        new_transfer.SetAssetId(asset_id);
        new_transfer.SetAssetType(RexAT_Texture);
        texture_transfers_[asset_id] = new_transfer;
    
        AssetModule::LogInfo("Requesting texture " + asset_id.ToString());

        NetOutMessage *m = net_interface_->StartMessageBuilding(RexNetMsgRequestImage);
        assert(m);
        
        m->AddUUID(client.agentID);
        m->AddUUID(client.sessionID);
        
        m->SetVariableBlockCount(1);
        m->AddUUID(asset_id); // Image UUID
        m->AddS8(0); // Discard level
        m->AddF32(100.0); // Download priority
        m->AddU32(0); // Starting packet
        m->AddU8(RexIT_Normal); // Image type
        
        net_interface_->FinishMessageBuilding(m);
    }
    
    void AssetManager::RequestOtherAsset(const RexUUID& asset_id, Core::uint asset_type)
    {
        // Asset transfers are keyed by transfer id, not asset id, so have to search in a bit cumbersome way
        AssetTransferMap::const_iterator i = asset_transfers_.begin();
        while (i != asset_transfers_.end())
        {
            if (i->second.GetAssetId() == asset_id)
            {
                AssetModule::LogWarning("Asset " + asset_id.ToString() + " already requested");
                return;
            }
            ++i;
        }

        RexUUID transfer_id;
        transfer_id.Random();
        
        AssetTransfer new_transfer;
        new_transfer.SetAssetId(asset_id);
        new_transfer.SetAssetType(asset_type);
        asset_transfers_[transfer_id] = new_transfer;
        
        AssetModule::LogInfo("Requesting asset " + asset_id.ToString());
        
        NetOutMessage *m = net_interface_->StartMessageBuilding(RexNetMsgTransferRequest);
        assert(m);
        
        m->AddUUID(transfer_id); // Transfer ID
        m->AddS32(RexAC_Asset); // Asset channel type
        m->AddS32(RexAS_Asset); // Asset source type
        m->AddF32(100.0); // Download priority
        
        Core::u8 asset_info[20]; // Asset info block with UUID and type
        memcpy(&asset_info[0], &asset_id.data, 16);
        memcpy(&asset_info[16], &asset_type, 4);
        m->AddBuffer(20, asset_info);
        
        net_interface_->FinishMessageBuilding(m);
    }
    
    void AssetManager::HandleTextureHeader(NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        AssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogWarning("Data received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }
        
        AssetTransfer& transfer = i->second;
        
        Core::u8 codec = msg->ReadU8();
        Core::u32 size = msg->ReadU32();
        Core::u16 packets = msg->ReadU16();
        
        transfer.SetSize(size);
        
        Core::uint data_size; 
        const Core::u8* data = msg->ReadBuffer(&data_size); // ImageData block
        transfer.ReceiveData(0, data, data_size);
        
        //AssetModule::LogInfo("First packet received for " + asset_id.ToString() + ", " + Core::ToString<Core::u16>(data_size) + " bytes");
        
        if (transfer.Ready())
        {
            StoreAsset(transfer);
            texture_transfers_.erase(i);
        }
    }
    
    void AssetManager::HandleTextureData(NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        AssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogWarning("Data received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }
        
        AssetTransfer& transfer = i->second;
        
        Core::u16 packet_index = msg->ReadU16();
        
        Core::uint data_size; 
        const Core::u8* data = msg->ReadBuffer(&data_size); // ImageData block
        transfer.ReceiveData(packet_index, data, data_size);
        
        //AssetModule::LogInfo("Packet " + Core::ToString<Core::u16>(packet_index) + " received for " + asset_id.ToString() + ", " + Core::ToString<Core::u16>(data_size) + " bytes");
        
        if (transfer.Ready())
        {
            StoreAsset(transfer);
            texture_transfers_.erase(i);
        }
    }
    
    void AssetManager::HandleTextureCancel(NetInMessage* msg)
    {
        RexUUID asset_id = msg->ReadUUID();
        AssetTransferMap::iterator i = texture_transfers_.find(asset_id);
        if (i == texture_transfers_.end())
        {
            AssetModule::LogWarning("Cancel received for nonexisting texture transfer " + asset_id.ToString());
            return;
        }

        AssetModule::LogInfo("Cancel received for texture transfer " + asset_id.ToString());
        texture_transfers_.erase(i);
    }
    
    void AssetManager::HandleAssetHeader(NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        AssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogWarning("Data received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }
        
        AssetTransfer& transfer = i->second;
        
        Core::s32 channel_type = msg->ReadS32();
        Core::s32 target_type = msg->ReadS32();
        Core::s32 status = msg->ReadS32();
        Core::s32 size = msg->ReadS32();
        
        transfer.SetSize(size);
        
        // We may get data packets before header, so check if all already received
        if (transfer.Ready())
        {
            StoreAsset(transfer);
            asset_transfers_.erase(i);
        }
    }
    
    void AssetManager::HandleAssetData(NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        AssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogWarning("Data received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }
        
        AssetTransfer& transfer = i->second;
        
        Core::s32 channel_type = msg->ReadS32();
        Core::s32 packet_index = msg->ReadS32();
        Core::s32 status = msg->ReadS32();
        
        Core::uint data_size; 
        const Core::u8* data = msg->ReadBuffer(&data_size); // Data block
        transfer.ReceiveData(packet_index, data, data_size);
        
        //AssetModule::LogInfo("Packet " + Core::ToString<Core::u16>(packet_index) + " received for " + transfer.GetAssetId().ToString() + ", " + Core::ToString<Core::u16>(data_size) + " bytes");
        
        if (transfer.Ready())
        {
            StoreAsset(transfer);
            asset_transfers_.erase(i);
        }
    }
    
    void AssetManager::HandleAssetCancel(NetInMessage* msg)
    {
        RexUUID transfer_id = msg->ReadUUID();
        AssetTransferMap::iterator i = asset_transfers_.find(transfer_id);
        if (i == asset_transfers_.end())
        {
            AssetModule::LogWarning("Cancel received for nonexisting asset transfer " + transfer_id.ToString());
            return;
        }
        
        AssetModule::LogInfo("Cancel received for asset transfer " + transfer_id.ToString());
        asset_transfers_.erase(i);
    }
    
    void AssetManager::StoreAsset(AssetTransfer& transfer)
    {
        const RexUUID& asset_id = transfer.GetAssetId();
        
        AssetModule::LogInfo("Storing complete asset " + asset_id.ToString());

        assets_[asset_id].asset_type_ = transfer.GetAssetType();
        assets_[asset_id].data_.resize(transfer.GetReceived());
        transfer.AssembleData(&assets_[asset_id].data_[0]);
    }
}