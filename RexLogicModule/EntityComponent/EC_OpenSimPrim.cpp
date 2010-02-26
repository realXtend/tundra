// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EntityComponent/EC_OpenSimPrim.h"
#include "RexLogicModule.h"
#include "ModuleManager.h"
#include <QStringList>

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
    State = 0;
    CRC = 0;
    TimeDilation = 0;
    PCode = 0;
    Material = 3;
    ClickAction = 0;
    UpdateFlags = 0;

    ServerScriptClass = "";

    SoundID = "";
    SoundVolume = 0;
    SoundRadius = 0;
    TextureAnimBuffer = 0;
    NameValueBuffer = 0;
    TextColor = 0;
    DataBuffer = 0;
    SelectPriority = 0;

    Sound.SetNull();
    OwnerID.SetNull();

    Gain= 0;
    Radius= 0;

    JointType= 0;
    Flags= 0;

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
    
    PrimDefaultRepeatU = 1.0;
    PrimDefaultRepeatV = 1.0;
    PrimDefaultOffsetU = 0.0;
    PrimDefaultOffsetV = 0.0;
    PrimDefaultUVRotation = 0.0;
}

EC_OpenSimPrim::~EC_OpenSimPrim()
{
}

QVariantMap EC_OpenSimPrim::getMaterials()
{
    QVariantMap qvmap;

    RexLogic::MaterialMap::const_iterator i = Materials.begin();
    for(int n = 0; i != Materials.end(); n++)
    {
        //if (i->second.Type == RexTypes::RexAT_Texture)
        //{    
        QString str;
        QString type;
        QStringList strlist;

        str.setNum(i->first);
        type.setNum(i->second.Type);
        strlist.append(type);
        strlist.append(i->second.asset_id.c_str());
        qvmap[str] = strlist;
        //}

        ++i;
    }
    return qvmap;
}

void EC_OpenSimPrim::setMaterials(QVariantMap qvmap)
{
    QVariantMap::Iterator it = qvmap.begin();
    while(it != qvmap.end())
    {
		QString key = it.key();
        QStringList strlist = it.value().toStringList();
		MaterialData data;
		data.Type = strlist[0].toUInt();
		data.asset_id = strlist[1].toStdString();
		Materials[key.toUInt()] = data;
        ++it;
    }
	//RexLogic::SendRexPrimData(LocalId);
	RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        rexlogic_->SendRexPrimData(LocalId);
	}
}

void EC_OpenSimPrim::setServerScriptClass(QString scriptclass)
{
    ServerScriptClass = scriptclass.toStdString();
 
	RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(GetFramework()->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
    if (rexlogic_)
    {
        rexlogic_->SendRexPrimData(LocalId);
	}
}

#ifdef _DEBUG
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
#endif

} // namespace RexLogic
