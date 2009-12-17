// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EntityComponent/EC_OpenSimPrim.h"
#include "RexLogicModule.h"

namespace RexLogic
{

EC_OpenSimPrim::EC_OpenSimPrim(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework())
{
    RegionHandle = 0;
    LocalId = 0;
    FullId.SetNull();
    ParentId = 0; 

    ObjectName = "";
    Description = "";
    HoveringText = "";
    MediaUrl = "";

    Material = 3;
    ClickAction = 0;
    UpdateFlags = 0;

    ServerScriptClass = "";

    SoundID = "";
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

    MeshID = "";
    CollisionMeshID = "";
    ParticleScriptID = "";

    AnimationPackageID = "";
    AnimationName = "";
    AnimationRate = 0;

    Materials.clear();

    PathCurve = 16;
    ProfileCurve = 1;
    PathBegin = 0;
    PathEnd = 0;
    PathScaleX = 1;
    PathScaleY = 1;
    PathShearX = 0;
    PathShearY = 0;
    PathTwist = 0;
    PathTwistBegin = 0;
    PathRadiusOffset = 0;
    PathTaperX = 0;
    PathTaperY = 0;
    PathRevolutions = 1;
    PathSkew = 0;
    ProfileBegin = 0;
    ProfileEnd = 0;
    ProfileHollow = 0;
}

EC_OpenSimPrim::~EC_OpenSimPrim()
{
}

void EC_OpenSimPrim::PrintDebug()
{
    RexLogicModule::LogInfo("*** EC_OpenSimPrim ***");
    RexLogicModule::LogInfo("LocalId:" + ToString(LocalId));
    RexLogicModule::LogInfo("RegionHandle:" + ToString(RegionHandle));
    RexLogicModule::LogInfo("LocalId:" + ToString(LocalId));
    RexLogicModule::LogInfo("FullId:" + FullId.ToString());
    RexLogicModule::LogInfo("ParentId:" + ToString(ParentId));

    RexLogicModule::LogInfo("ObjectName:" + ObjectName);
    RexLogicModule::LogInfo("Description:" + Description);
    RexLogicModule::LogInfo("HoveringText:" + HoveringText);
    RexLogicModule::LogInfo("MediaUrl:" + MediaUrl);
    
    RexLogicModule::LogInfo("Material:" + ToString((uint)Material));
    RexLogicModule::LogInfo("ClickAction:" + ToString((uint)ClickAction));
    RexLogicModule::LogInfo("UpdateFlags:" + ToString(UpdateFlags));
    
    RexLogicModule::LogInfo("ServerScriptClass:" + ServerScriptClass);
    RexLogicModule::LogInfo("CollisionMeshID:" + CollisionMeshID);

    RexLogicModule::LogInfo("SoundID:" + SoundID);
    RexLogicModule::LogInfo("SoundVolume:" + ToString(SoundVolume));
    RexLogicModule::LogInfo("SoundRadius:" + ToString(SoundRadius));

    RexLogicModule::LogInfo("SelectPriority:" + ToString(SelectPriority));
}

} // namespace RexLogic
