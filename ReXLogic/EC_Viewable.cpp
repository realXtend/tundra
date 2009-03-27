// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Viewable.h"

namespace RexLogic
{
    EC_Viewable::EC_Viewable(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {

    }

    EC_Viewable::~EC_Viewable()
    {
    }
    
    void EC_Viewable::HandleRexPrimData(OpenSimProtocol::NetworkEventInboundData* data)
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

        int idx = 0;
        DrawType = fulldata[idx];
        idx += sizeof(uint8_t);
        IsVisible = *(bool*)(&fulldata[idx]);
        idx += sizeof(bool);            
        CastShadows = *(bool*)(&fulldata[idx]);
        idx += sizeof(bool);  
        LightCreatesShadows = *(bool*)(&fulldata[idx]);
        idx += sizeof(bool);  
        DescriptionTexture = *(bool*)(&fulldata[idx]);
        idx += sizeof(bool);      
        ScaleToPrim = *(bool*)(&fulldata[idx]);
        idx += sizeof(bool);    

        DrawDistance = *(float*)(&fulldata[idx]);
        idx += sizeof(float);
        LOD = *(float*)(&fulldata[idx]);
        idx += sizeof(float);

        MeshUUID = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);
        
        // Read collisionmesh, but don't use it
        RexUUID tempcollisionmesh = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);        
        
        ParticleScriptUUID = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);
        AnimationPackageUUID = *(RexUUID*)(&fulldata[idx]);
        idx += sizeof(RexUUID);        
        
        std::string AnimationName = "";
        uint8_t readbyte = fulldata[idx];
        idx++;
        while(readbyte != 0)
        {
            AnimationName.push_back((char)readbyte);
            readbyte = fulldata[idx];
            idx++;    
        }  
         
        AnimationRate = *(float*)(&fulldata[idx]);
        idx += sizeof(float);         
        
        // Materials
        Materials.clear();
        uint8_t tempmaterialcount = fulldata[idx];
        idx++;        

        uint8_t tempmaterialindex = 0;        
        for(int i=0;i<tempmaterialcount;i++)
        {
            MaterialData newmaterialdata;

            newmaterialdata.Type = fulldata[idx];
            idx++;
            newmaterialdata.UUID = *(RexUUID*)(&fulldata[idx]);
            idx += sizeof(RexUUID);
            tempmaterialindex = fulldata[idx];
            idx++;

            Materials[tempmaterialindex] = newmaterialdata;                           
        }
        
        delete fulldata;    
    }
}
