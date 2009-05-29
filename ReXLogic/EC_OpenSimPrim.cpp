// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimPrim.h"
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

        ServerScriptClass = "";

        SoundUUID;
        SoundVolume = 0;
        SoundRadius = 0;
        
        SelectPriority = 0;

        DrawType = RexTypes::DRAWTYPE_PRIM;
        IsVisible = true;
        CastShadows = false;
        LightCreatesShadows = false;
        DescriptionTexture = false;
        ScaleToPrim = true;
        DrawDistance = 0;
        LOD = 0;
        
        HasPrimShapeData = false;
        
        MeshUUID.SetNull();
        CollisionMesh.SetNull();
        ParticleScriptUUID.SetNull();

        AnimationPackageUUID.SetNull();
        AnimationName = "";
        AnimationRate = 0;

        Materials.clear();
    }

    EC_OpenSimPrim::~EC_OpenSimPrim()
    {
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
        
        RexLogicModule::LogInfo("ServerScriptClass:" + ServerScriptClass);        
        RexLogicModule::LogInfo("CollisionMesh:" + CollisionMesh.ToString());

        RexLogicModule::LogInfo("SoundUUID:" + SoundUUID.ToString());        
        RexLogicModule::LogInfo("SoundVolume:" + Core::ToString(SoundVolume));
        RexLogicModule::LogInfo("SoundRadius:" + Core::ToString(SoundRadius));        

        RexLogicModule::LogInfo("SelectPriority:" + Core::ToString(SelectPriority));    
    }
}
