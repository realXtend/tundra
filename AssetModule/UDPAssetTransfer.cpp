#include "StableHeaders.h"
#include "UDPAssetTransfer.h"
#include "AssetModule.h"

namespace Asset
{
    UDPAssetTransfer::UDPAssetTransfer() :
        size_(0),
        received_(0),
        time_(0.0)
    {
    }
    
    UDPAssetTransfer::~UDPAssetTransfer()
    {
    }
    
    bool UDPAssetTransfer::Ready() const
    {
        if (!size_) 
            return false; // No header received, size not known yet
        
        return received_ >= size_;
    }
    
    uint UDPAssetTransfer::GetReceivedContinuous() const
    {
        uint size = 0;
        
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        uint expected_index = 0;
      
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
    
    void UDPAssetTransfer::ReceiveData(uint packet_index, const u8* data, uint size)
    {
        time_ = 0.0;
        
        if (!size)
        {
            AssetModule::LogDebug("Trying to store zero bytes of data");
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
            AssetModule::LogDebug("Already received asset data packet index " + ToString<uint>(packet_index));
        }
    }
    
    void UDPAssetTransfer::AssembleData(u8* buffer) const
    {
        DataPacketMap::const_iterator i = data_packets_.begin();
        
        uint expected_index = 0;
      
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
}