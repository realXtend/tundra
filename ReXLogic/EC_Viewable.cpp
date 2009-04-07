// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Viewable.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    EC_Viewable::EC_Viewable(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        DrawType = 0;
        IsVisible = true;
        CastShadows = false;
        LightCreatesShadows = false;
        DescriptionTexture = false;
        ScaleToPrim = true;
        
        DrawDistance = 0;
        LOD = 0;
        
        MeshUUID.SetNull();
        ParticleScriptUUID.SetNull();

        AnimationPackageUUID.SetNull();
        AnimationName = "";
        AnimationRate = 0;

        Materials.clear();
    }

    EC_Viewable::~EC_Viewable()
    {
    }
    
    void EC_Viewable::HandleRexPrimData(const uint8_t* primdata)
    {
        int idx = 0;
        DrawType = primdata[idx];
        idx += sizeof(uint8_t);
        IsVisible = *(bool*)(&primdata[idx]);
        idx += sizeof(bool);            
        CastShadows = *(bool*)(&primdata[idx]);
        idx += sizeof(bool);  
        LightCreatesShadows = *(bool*)(&primdata[idx]);
        idx += sizeof(bool);  
        DescriptionTexture = *(bool*)(&primdata[idx]);
        idx += sizeof(bool);      
        ScaleToPrim = *(bool*)(&primdata[idx]);
        idx += sizeof(bool);    

        DrawDistance = *(float*)(&primdata[idx]);
        idx += sizeof(float);
        LOD = *(float*)(&primdata[idx]);
        idx += sizeof(float);

        MeshUUID = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);
        
        // Read collisionmesh, but don't use it
        RexUUID tempcollisionmesh = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);        
        
        ParticleScriptUUID = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);
        AnimationPackageUUID = *(RexUUID*)(&primdata[idx]);
        idx += sizeof(RexUUID);        
        
        std::string AnimationName = "";
        uint8_t readbyte = primdata[idx];
        idx++;
        while(readbyte != 0)
        {
            AnimationName.push_back((char)readbyte);
            readbyte = primdata[idx];
            idx++;    
        }  
         
        AnimationRate = *(float*)(&primdata[idx]);
        idx += sizeof(float);         
        
        // Materials
        Materials.clear();
        uint8_t tempmaterialcount = primdata[idx];
        idx++;        

        uint8_t tempmaterialindex = 0;        
        for(int i=0;i<tempmaterialcount;i++)
        {
            MaterialData newmaterialdata;

            newmaterialdata.Type = primdata[idx];
            idx++;
            newmaterialdata.UUID = *(RexUUID*)(&primdata[idx]);
            idx += sizeof(RexUUID);
            tempmaterialindex = primdata[idx];
            idx++;

            Materials[tempmaterialindex] = newmaterialdata;                           
        }   
    }
    
    void EC_Viewable::PrintDebug()
    {
        RexLogicModule::LogInfo("*** EC_Viewable ***");
        RexLogicModule::LogInfo("DrawType:" + Core::ToString((Core::uint)DrawType));  
        RexLogicModule::LogInfo("IsVisible:" + Core::ToString(IsVisible));  
        RexLogicModule::LogInfo("CastShadows:" + Core::ToString(CastShadows));      
        RexLogicModule::LogInfo("LightCreatesShadows:" + Core::ToString(LightCreatesShadows));    
        RexLogicModule::LogInfo("DescriptionTexture:" + Core::ToString(DescriptionTexture));    
        RexLogicModule::LogInfo("ScaleToPrim:" + Core::ToString(ScaleToPrim)); 
        
        RexLogicModule::LogInfo("DrawDistance:" + Core::ToString(DrawDistance));         
        RexLogicModule::LogInfo("LOD:" + Core::ToString(LOD));    
  
        RexLogicModule::LogInfo("MeshUUID:" + MeshUUID.ToString());    
        RexLogicModule::LogInfo("ParticleScriptUUID:" + ParticleScriptUUID.ToString());  

        RexLogicModule::LogInfo("AnimationPackageUUID:" + AnimationPackageUUID.ToString());  
        RexLogicModule::LogInfo("AnimationName:" + AnimationName);          
        RexLogicModule::LogInfo("AnimationRate:" + Core::ToString(AnimationRate));   

        RexLogicModule::LogInfo("MaterialCount:" + Core::ToString(Materials.size())); 
        MaterialMap::iterator it = Materials.begin();
        for(MaterialMap::const_iterator iter = Materials.begin(); iter != Materials.end(); ++iter)
            RexLogicModule::LogInfo(Core::ToString((Core::uint)iter->first) + " " + Core::ToString((Core::uint)iter->second.Type) + " " + iter->second.UUID.ToString());
    }
}
