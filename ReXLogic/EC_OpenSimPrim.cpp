// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimPrim.h"
#include "NetInMessage.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    EC_OpenSimPrim::EC_OpenSimPrim(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
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
        msg->SkipToNextVariable();      // TimeDilation U16
        LocalId = msg->ReadU32();
        msg->SkipToNextVariable();		// State U8
        FullId = msg->ReadUUID();
        
        msg->SkipToNextVariable();		// CRC U32
        msg->SkipToNextVariable();      // PCode  
        
        Material = msg->ReadU8();
        ClickAction = msg->ReadU8();
        Scale = msg->ReadVector3();
        
        const uint8_t *objectdatabytes = msg->ReadBuffer(&bytes_read);
        if(bytes_read == 60)
        {
            Position = *(Core::Vector3df*)(&objectdatabytes[0]);
            Rotation = *(Core::Quaternion*)(&objectdatabytes[36]);
        }
        else
            RexLogicModule::LogError("Error reading ObjectData for prim:" + Core::ToString(LocalId) + ". Bytes read:" + Core::ToString(bytes_read));    
        
        ParentId = msg->ReadU32();
        UpdateFlags = msg->ReadU32();
        
        // Skip path related variables

        msg->SkipToFirstVariableByName("Text");
        HoveringText = (const char *)msg->ReadBuffer(&bytes_read); 
        msg->SkipToNextVariable();      // TextColor
        MediaUrl = (const char *)msg->ReadBuffer(&bytes_read);     
    }
    
    void EC_OpenSimPrim::HandleRexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        size_t bytes_read;    
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("Parameter");           
        
        const uint8_t *readbytedata;      
        readbytedata = data->message->ReadBuffer(&bytes_read);
        RexUUID primuuid = *(RexUUID*)(&readbytedata[0]);      
        
        size_t fulldatasize = 0;        
        NetVariableType nextvartype = data->message->CheckNextVariableType();
        while(nextvartype != NetVarNone)
        {
            data->message->ReadBuffer(&bytes_read);
            fulldatasize += bytes_read;
            nextvartype = data->message->CheckNextVariableType();
        }
        
        data->message->ResetReading();
        data->message->SkipToFirstVariableByName("Parameter");
        data->message->SkipToNextVariable();            // Prim UUID
        
        uint8_t *fulldata = new uint8_t[fulldatasize];
        int pos = 0;
        nextvartype = data->message->CheckNextVariableType();
        while(nextvartype != NetVarNone)
        {
            readbytedata = data->message->ReadBuffer(&bytes_read);
            memcpy(fulldata+pos,readbytedata,bytes_read);
            pos += bytes_read;
            nextvartype = data->message->CheckNextVariableType();
        }

        // Skip
        int idx = 0;
        idx += sizeof(uint8_t);     // DrawType
        idx += sizeof(bool);        // IsVisible
        idx += sizeof(bool);        // CastShadows  
        idx += sizeof(bool);        // LightCreatesShadows  
        idx += sizeof(bool);        // DescriptionTexture      
        idx += sizeof(bool);        // ScaleToPrim    
        idx += sizeof(float);       // DrawDistance
        idx += sizeof(float);       // LOD
        idx += sizeof(RexUUID);     // MeshUUID
        
        CollisionMesh = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);        
        
        idx += sizeof(RexUUID);     // ParticleScriptUUID
        idx += sizeof(RexUUID);     // AnimationPackageUUID       
        
        while(fulldata[idx] != 0)
            idx++;    

        idx++;      

        idx += sizeof(float);       // AnimationRate         
        
        uint8_t tempmaterialcount = fulldata[idx];
        idx++;        
        for(int i=0;i<tempmaterialcount;i++)
        {
            idx++;                  // type
            idx += sizeof(RexUUID); // uuid
            idx++;                  // index                             
        } 

        ServerScriptClass = "";
        uint8_t readbyte = fulldata[idx];
        idx++;
        while(readbyte != 0)
        {
            ServerScriptClass.push_back((char)readbyte);
            readbyte = fulldata[idx];
            idx++;    
        }  
  
        SoundUUID = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);        
        SoundVolume = *(float*)(&fulldata[idx]);
        idx += sizeof(float);
        SoundRadius = *(float*)(&fulldata[idx]);
        idx += sizeof(float);                 
        
        SelectPriority = *(uint32_t*)(&fulldata[idx]);
        idx += sizeof(uint32_t);    
 
        delete fulldata;    
    }    
    
    void EC_OpenSimPrim::HandleObjectName(OpenSimProtocol::NetworkEventInboundData* data)
    {    
        size_t bytes_read;
    
        data->message->SkipToFirstVariableByName("Name");
        ObjectName = (const char *)data->message->ReadBuffer(&bytes_read); 
    }
    
    void EC_OpenSimPrim::HandleObjectDescription(OpenSimProtocol::NetworkEventInboundData* data)
    {    
        size_t bytes_read;
    
        data->message->SkipToFirstVariableByName("Description");
        Description = (const char *)data->message->ReadBuffer(&bytes_read); 
    }    
}
