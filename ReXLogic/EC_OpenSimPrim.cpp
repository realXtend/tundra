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
        FullId.SetNull();
        ParentId = 0; 
        
        ObjectName = "";
        Description = "";
        HoveringText = "";
        MediaUrl = "";

        Material = 0;
        ClickAction = 0;
        UpdateFlags = 0;

        Position = Core::Vector3df(0,0,0);
        Rotation = Core::Quaternion(0,0,0,0);
        Scale = Core::Vector3df(0,0,0);

        ServerScriptClass = "";
        
        CollisionMesh.SetNull();
        
        SoundUUID;
        SoundVolume = 0;
        SoundRadius = 0;
        
        SelectPriority = 0;
    }

    EC_OpenSimPrim::~EC_OpenSimPrim()
    {
    }
    
    void EC_OpenSimPrim::HandleObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data)
    {/*
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
        HoveringText = msg->ReadString();
        msg->SkipToNextVariable();      // TextColor
        MediaUrl = msg->ReadString();    
        */
    }
    
    void EC_OpenSimPrim::HandleRexPrimData(const uint8_t* primdata)
    {
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
        
        CollisionMesh = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);        
        
        idx += sizeof(RexUUID);     // ParticleScriptUUID
        idx += sizeof(RexUUID);     // AnimationPackageUUID       
        
        while(primdata[idx] != 0)   // AnimationName
            idx++;    

        idx++;                      // AnimationName 0 byte

        idx += sizeof(float);       // AnimationRate         
        
        uint8_t tempmaterialcount = primdata[idx];
        idx++;        
        for(int i=0;i<tempmaterialcount;i++)
        {
            idx++;                  // type
            idx += sizeof(RexUUID); // uuid
            idx++;                  // index                             
        } 

        ServerScriptClass = "";
        uint8_t readbyte = primdata[idx];
        idx++;
        while(readbyte != 0)
        {
            ServerScriptClass.push_back((char)readbyte);
            readbyte = primdata[idx];
            idx++;    
        }  
  
        SoundUUID = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);        
        SoundVolume = *(float*)(&primdata[idx]);
        idx += sizeof(float);
        SoundRadius = *(float*)(&primdata[idx]);
        idx += sizeof(float);                 
        
        SelectPriority = *(uint32_t*)(&primdata[idx]);
        idx += sizeof(uint32_t);
    }    
    
    void EC_OpenSimPrim::HandleObjectName(OpenSimProtocol::NetworkEventInboundData* data)
    {    
        data->message->SkipToFirstVariableByName("Name");
        ObjectName = data->message->ReadString();
    }
    
    void EC_OpenSimPrim::HandleObjectDescription(OpenSimProtocol::NetworkEventInboundData* data)
    {        
        data->message->SkipToFirstVariableByName("Description");
        Description = data->message->ReadString();
    }
    
    void EC_OpenSimPrim::PrintDebug()
    {
        RexLogicModule::LogInfo("*** EC_OpenSimPrim ***");
        RexLogicModule::LogInfo("LocalId:" + Core::ToString(LocalId));
        RexLogicModule::LogInfo("RegionHandle:" + Core::ToString(RegionHandle));    
        RexLogicModule::LogInfo("LocalId:" + Core::ToString(LocalId));
        RexLogicModule::LogInfo("FullId:" + FullId.ToString());
        RexLogicModule::LogInfo("ParentId:" + Core::ToString(ParentId));        

        RexLogicModule::LogInfo("ObjectName:" + ObjectName);
        RexLogicModule::LogInfo("Description:" + Description);
        RexLogicModule::LogInfo("HoveringText:" + HoveringText);
        RexLogicModule::LogInfo("MediaUrl:" + MediaUrl);
        
        RexLogicModule::LogInfo("Material:" + Core::ToString((Core::uint)Material));
        RexLogicModule::LogInfo("ClickAction:" + Core::ToString((Core::uint)ClickAction));
        RexLogicModule::LogInfo("UpdateFlags:" + Core::ToString(UpdateFlags));
        
        RexLogicModule::LogInfo("Position:" + Core::ToString(Position));        
        RexLogicModule::LogInfo("Rotation:" + Core::ToString(Rotation)); 
        RexLogicModule::LogInfo("Scale:" + Core::ToString(Scale));

        RexLogicModule::LogInfo("ServerScriptClass:" + ServerScriptClass);        
        RexLogicModule::LogInfo("CollisionMesh:" + CollisionMesh.ToString());

        RexLogicModule::LogInfo("SoundUUID:" + SoundUUID.ToString());        
        RexLogicModule::LogInfo("SoundVolume:" + Core::ToString(SoundVolume));
        RexLogicModule::LogInfo("SoundRadius:" + Core::ToString(SoundRadius));        

        RexLogicModule::LogInfo("SelectPriority:" + Core::ToString(SelectPriority));    
    }   
}
