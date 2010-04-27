/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_OpenSimPrim.cpp
 *  @brief  EC_OpenSimPrim is a data data container class for primitive objects in OpenSim.
 */

#include "StableHeaders.h"
#include "EC_OpenSimPrim.h"
#include "ModuleInterface.h"
#include "SceneManager.h"

#include <Poco/Logger.h>

#define LogInfo(msg) Poco::Logger::get("EC_OpenSimPrim").information(msg);

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
    MaterialMap::const_iterator i = Materials.begin();
    for(int n = 0; i != Materials.end(); n++)
    {
        //if (i->second.Type == RexTypes::RexAT_Texture)
        //{    
        QString str, type;
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

void EC_OpenSimPrim::setMaterials(QVariantMap &qvmap)
{
    QMapIterator<QString, QVariant> it(qvmap);
    while(it.hasNext())
    {
        it.next();
        QString key = it.key();
        QStringList strlist = it.value().toStringList();
        MaterialData data;
        data.Type = strlist[0].toUInt();
        data.asset_id = strlist[1].toStdString();
        Materials[key.toUInt()] = data;
    }
}

QStringList EC_OpenSimPrim::GetChildren()
{
    QStringList prim_children;
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        EC_OpenSimPrim *prim = entity.GetComponent<EC_OpenSimPrim>().get();
        if (prim && prim->getParentId() == getLocalId())
        {
            QString id;
            id.setNum(prim->getLocalId());
            prim_children.append(id);
        }
    }

    return prim_children;
}

#ifdef _DEBUG
void EC_OpenSimPrim::PrintDebug()
{
    LogInfo("LocalId:" + ToString(LocalId));
    LogInfo("RegionHandle:" + ToString(RegionHandle));
    LogInfo("LocalId:" + ToString(LocalId));
    LogInfo("FullId:" + FullId.ToString());
    LogInfo("ParentId:" + ToString(ParentId));

    LogInfo("ObjectName:" + ObjectName);
    LogInfo("Description:" + Description);
    LogInfo("HoveringText:" + HoveringText);
    LogInfo("MediaUrl:" + MediaUrl);
    
    LogInfo("Material:" + ToString((uint)Material));
    LogInfo("ClickAction:" + ToString((uint)ClickAction));
    LogInfo("UpdateFlags:" + ToString(UpdateFlags));
    
    LogInfo("ServerScriptClass:" + ServerScriptClass);
    LogInfo("CollisionMeshID:" + CollisionMeshID);

    LogInfo("SoundID:" + SoundID);
    LogInfo("SoundVolume:" + ToString(SoundVolume));
    LogInfo("SoundRadius:" + ToString(SoundRadius));

    LogInfo("SelectPriority:" + ToString(SelectPriority));
}
#endif

