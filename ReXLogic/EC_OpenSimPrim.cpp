// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimPrim.h"
#include "NetInMessage.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    EC_OpenSimPrim::EC_OpenSimPrim(Foundation::ModuleInterface* module)
    {
        RegionHandle = 0;
        LocalId = 0;
        
        ObjectName = "";
        Description = "";
    }

    EC_OpenSimPrim::~EC_OpenSimPrim()
    {
    }
    
    void EC_OpenSimPrim::HandleObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {
        size_t bytes_read;
        NetInMessage *msg = data->message;    
    
        msg->ResetReading();    
        RegionHandle = msg->ReadU64();
        msg->SkipToNextVariable(); // TimeDilation U16
        LocalId = msg->ReadU32();
        msg->SkipToNextVariable();		// State U8
        FullId = msg->ReadUUID();
        
        msg->SkipToNextVariable();		// CRC U32
        msg->SkipToNextVariable();      // PCode  
        
        Material = msg->ReadU8();
        ClickAction = msg->ReadU8();
        Scale =  msg->ReadVector3(); // TODO tucofixme, Scale = msg->ReadVector3();
        msg->SkipToNextVariable();      // ObjectData
        
        ParentId = msg->ReadU32();
        UpdateFlags = msg->ReadU32();
        
        // Skip path related variables

        msg->SkipToFirstVariableByName("Text");
        HoveringText = (const char *)msg->ReadBuffer(&bytes_read); 
        msg->SkipToNextVariable();      // TextColor
        MediaUrl = (const char *)msg->ReadBuffer(&bytes_read);     
    }
}
