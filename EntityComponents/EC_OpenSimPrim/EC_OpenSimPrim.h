/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_OpenSimPrim.h
 *  @brief  EC_OpenSimPrim is a data data container class for primitive objects in OpenSim.
 */

#ifndef incl_EC_OpenSimPrim_EC_OpenSimPrim_h
#define incl_EC_OpenSimPrim_EC_OpenSimPrim_h

#include "ComponentInterface.h"
#include "RexUUID.h"
#include "Color.h"
#include "Declare_EC.h"

#include <QVariant>
#include <QStringList>
#include <QDebug>

class QTimer;

//! Material data structure
struct MaterialData
{
    uint8_t Type;
    RexTypes::RexAssetID asset_id;
};

//! Map for holding materials
typedef std::map<uint8_t, MaterialData> MaterialMap;

//! Map for holding prim textures
typedef std::map<uint8_t, RexTypes::RexAssetID> TextureMap;

//! Map for holding prim face colors
typedef std::map<uint8_t, Color> ColorMap;

//! Map for holding prim face material bytes
typedef std::map<uint8_t, uint8_t> MaterialTypeMap;

//! Map for holding prim face uv parameter info
typedef std::map<uint8_t, Real> UVParamMap;

//! Each scene entity representing a prim in OpenSim sense has
//! this component.
class EC_OpenSimPrim : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_OpenSimPrim);

    Q_OBJECT
    Q_PROPERTY(QString ObjectName READ getObjectName WRITE setObjectName DESIGNABLE false)
    Q_PROPERTY(QString FullId READ getFullId DESIGNABLE false)
    Q_PROPERTY(QString ServerScriptClass READ getServerScriptClass WRITE setServerScriptClass)
    Q_PROPERTY(QString Description READ getDescription WRITE setDescription)
    Q_PROPERTY(QString MediaUrl READ getMediaUrl WRITE setMediaUrl DESIGNABLE false) // not handled anywhere
    Q_PROPERTY(QString HoveringText READ getHoveringText WRITE setHoveringText)
    Q_PROPERTY(bool CastShadows READ getCastShadows WRITE setCastShadows)
    Q_PROPERTY(double SoundVolume READ getSoundVolume WRITE setSoundVolume)
    Q_PROPERTY(double SoundRadius READ getSoundRadius WRITE setSoundRadius)
    Q_PROPERTY(unsigned int Material READ getMaterial WRITE setMaterial DESIGNABLE false)
    Q_PROPERTY(unsigned int ClickAction READ getClickAction WRITE setClickAction)
    Q_PROPERTY(unsigned int PathCurve READ getPathCurve WRITE setPathCurve)
    Q_PROPERTY(unsigned int ProfileCurve READ getProfileCurve WRITE setProfileCurve)
    Q_PROPERTY(QVariant RegionHandle READ getRegionHandle WRITE setRegionHandle DESIGNABLE false)
    Q_PROPERTY(unsigned int LocalId READ getLocalId WRITE setLocalId DESIGNABLE false)

    Q_PROPERTY(unsigned int PrimDefaultMaterialType READ getPrimDefaultMaterialType WRITE setPrimDefaultMaterialType DESIGNABLE false)
    Q_PROPERTY(unsigned int DrawType READ getDrawType WRITE setDrawType)

    Q_PROPERTY(double DrawDistance READ getDrawDistance WRITE setDrawDistance)
    Q_PROPERTY(double LOD READ getLOD WRITE setLOD)

    Q_PROPERTY(QString CollisionMeshID READ getCollisionMeshID WRITE setCollisionMeshID)
    Q_PROPERTY(QString SoundID READ getSoundID WRITE setSoundID)
    Q_PROPERTY(QString MeshID READ getMeshID WRITE setMeshID)
    Q_PROPERTY(QString ParticleScriptID READ getParticleScriptID WRITE setParticleScriptID)
    Q_PROPERTY(QString AnimationPackageID READ getAnimationPackageID WRITE setAnimationPackageID)
    Q_PROPERTY(QString PrimDefaultTextureID READ getPrimDefaultTextureID WRITE setPrimDefaultTextureID)
    Q_PROPERTY(QString AnimationName READ getAnimationName WRITE setAnimationName)
    Q_PROPERTY(double AnimationRate READ getAnimationRate WRITE setAnimationRate)

    Q_PROPERTY(bool IsVisible READ getIsVisible WRITE setIsVisible)
    Q_PROPERTY(bool LightCreatesShadows READ getLightCreatesShadows WRITE setLightCreatesShadows)
    Q_PROPERTY(bool DescriptionTexture READ getDescriptionTexture WRITE setDescriptionTexture)
    Q_PROPERTY(bool ScaleToPrim READ getScaleToPrim WRITE setScaleToPrim)

    Q_PROPERTY(double PathBegin READ getPathBegin WRITE setPathBegin)
    Q_PROPERTY(double PathEnd READ getPathEnd WRITE setPathEnd)
    Q_PROPERTY(double PathScaleX READ getPathScaleX WRITE setPathScaleX)
    Q_PROPERTY(double PathScaleY READ getPathScaleY WRITE setPathScaleY)
    Q_PROPERTY(double PathShearX READ getPathShearX WRITE setPathShearX)
    Q_PROPERTY(double PathShearY READ getPathShearY WRITE setPathShearY)
    Q_PROPERTY(double PathTwist READ getPathTwist WRITE setPathTwist)
    Q_PROPERTY(double PathTwistBegin READ getPathTwistBegin WRITE setPathTwistBegin)
    Q_PROPERTY(double PathRadiusOffset READ getPathRadiusOffset WRITE setPathRadiusOffset)
    Q_PROPERTY(double PathTaperX READ getPathTaperX WRITE setPathTaperX)
    Q_PROPERTY(double PathTaperY READ getPathTaperY WRITE setPathTaperY)
    Q_PROPERTY(double PathRevolutions READ getPathRevolutions WRITE setPathRevolutions DESIGNABLE false)
    Q_PROPERTY(double PathSkew READ getPathSkew WRITE setPathSkew)
    Q_PROPERTY(double ProfileBegin READ getProfileBegin WRITE setProfileBegin)
    Q_PROPERTY(double ProfileEnd READ getProfileEnd WRITE setProfileEnd)
    Q_PROPERTY(double ProfileHollow READ getProfileHollow WRITE setProfileHollow)
    Q_PROPERTY(bool HasPrimShapeData READ getHasPrimShapeData DESIGNABLE false)
    Q_PROPERTY(QVariant SelectPriority READ getSelectPriority WRITE setSelectPriority)

    Q_PROPERTY(unsigned int UpdateFlags READ getUpdateFlags WRITE setUpdateFlags)
    Q_PROPERTY(unsigned int ParentId READ getParentId WRITE setParentId DESIGNABLE false)

    Q_PROPERTY(QVariantMap Materials READ getMaterials WRITE setMaterials DESIGNABLE false)

public:
    virtual ~EC_OpenSimPrim();

    QString getCollisionMeshID() const { return QString(CollisionMeshID.c_str()); }
    void setCollisionMeshID(QString value) { CollisionMeshID = value.toStdString(); }

    QString getSoundID() const { return QString(SoundID.c_str()); }
    void setSoundID(QString value) { SoundID = value.toStdString(); }

    QString getMeshID() const { return QString(MeshID.c_str()); }
    void setMeshID(QString value) { MeshID = value.toStdString(); DrawType = RexTypes::DRAWTYPE_MESH;}

    QString getParticleScriptID() const { return QString(ParticleScriptID.c_str()); }
    void setParticleScriptID(QString value) { ParticleScriptID = value.toStdString(); }

    QString getAnimationPackageID() const { return QString(AnimationPackageID.c_str()); }
    void setAnimationPackageID(QString value) { AnimationPackageID = value.toStdString(); }

    QString getPrimDefaultTextureID() const { return QString(PrimDefaultTextureID.c_str()); }
    void setPrimDefaultTextureID(QString value) { PrimDefaultTextureID = value.toStdString(); }

    QString getAnimationName() const { return QString(AnimationName.c_str()); }
    void setAnimationName(QString value) { AnimationName = value.toStdString(); }

    QString getObjectName() const { return QString(ObjectName.c_str()); }
    void setObjectName(QString name) { ObjectName = name.toStdString(); }

    QString getDescription() const { return QString(Description.c_str()); }
    void setDescription(QString value) { Description = value.toStdString(); }

    QString getMediaUrl() const { return QString(MediaUrl.c_str()); }
    void setMediaUrl(QString value) { MediaUrl = value.toStdString(); }

    QString getHoveringText() const { return QString(HoveringText.c_str()); }
    void setHoveringText(QString value) { HoveringText = value.toStdString(); }

    QString getServerScriptClass() const { return QString(ServerScriptClass.c_str()); }
    void setServerScriptClass(const QString &scriptclass) { ServerScriptClass = scriptclass.toStdString(); }

    bool getCastShadows() const { return CastShadows; }
    void setCastShadows(bool shadows) { CastShadows = shadows; }

    double getSoundVolume() const { return (double)SoundVolume; }
    void setSoundVolume(double sound) { SoundVolume = (float)sound; }

    double getSoundRadius() const { return (double)SoundRadius; }
    void setSoundRadius(double sound) { SoundRadius = (float)sound; }

    void setClickAction(unsigned int mat) { ClickAction = (uint8_t)mat; }
    unsigned int getClickAction() const { return ClickAction; }

    void setMaterial(unsigned int mat) { Material = (uint8_t)mat; }
    unsigned int getMaterial() const { return Material; }

    void setPrimDefaultMaterialType(unsigned int mat) { PrimDefaultMaterialType = (uint8_t)mat; }
    unsigned int getPrimDefaultMaterialType() const { return PrimDefaultMaterialType; }

    void setDrawType(unsigned int mat) { DrawType = (uint8_t)mat; }
    unsigned int getDrawType() const { return DrawType; }

    void setPathCurve(unsigned int value) { PathCurve = (uint8_t)value; }
    unsigned int getPathCurve() const { return PathCurve; }

    void setProfileCurve(unsigned int value) { ProfileCurve = (uint8_t)value; }
    unsigned int getProfileCurve() const { return ProfileCurve; }

    void setRegionHandle(const QVariant &reg) { RegionHandle = reg.toULongLong(); }
    QVariant getRegionHandle() const { return QVariant(static_cast<qulonglong>(RegionHandle)); }

    void setLocalId(unsigned int lid) { LocalId = (uint32_t)lid; }
    unsigned int getLocalId() const { return LocalId; }

    // PRIM SHAPE PROPERTIES

    // 0 to 1, quanta = 0.01
    double getPathBegin() const { return (double)PathBegin; }
    void setPathBegin(double value) { if(value>1) PathBegin=1; else if(value<0) PathBegin=0; else PathBegin = (float)value; }

    // 0 to 1, quanta = 0.01
    double getPathEnd()const  { return (double)PathEnd; }
    void setPathEnd(double value) { if(value>1) PathEnd=1; else if(value<0) PathEnd=0; else PathEnd = (float)value; }

    // 0 to 1, quanta = 0.01
    double getPathScaleX() const { return (double)PathScaleX; }
    void setPathScaleX(double value) { if(value>1) PathScaleX=1; else if(value<0) PathScaleX=0; else PathScaleX = (float)value; }

    // 0 to 1, quanta = 0.01
    double getPathScaleY() const { return (double)PathScaleY; }
    void setPathScaleY(double value) { if(value>1) PathScaleY=1; else if(value<0) PathScaleY=0; else PathScaleY = (float)value; }

    // -.5 to .5, quanta = 0.01
    double getPathShearX() const { return (double)PathShearX; }
    void setPathShearX(double value) { if(value>0.5) PathShearX=0.5; else if(value<-0.5) PathShearX=-0.5; else PathShearX = (float)value; }

    // -.5 to .5, quanta = 0.01
    double getPathShearY() const { return (double)PathShearY; }
    void setPathShearY(double value) { if(value>0.5) PathShearY=0.5; else if(value<-0.5) PathShearY=-0.5; else PathShearY = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathTwist() const { return (double)PathTwist; }
    void setPathTwist(double value) { if(value>1) PathTwist=1; else if(value<-1) PathTwist=-1; else PathTwist = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathTwistBegin() const { return (double)PathTwistBegin; }
    void setPathTwistBegin(double value) { if(value>1) PathTwistBegin=1; else if(value<-1) PathTwistBegin=-1; else PathTwistBegin = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathRadiusOffset() const { return (double)PathRadiusOffset; }
    void setPathRadiusOffset(double value) { if(value>1) PathRadiusOffset=1; else if(value<-1) PathRadiusOffset=-1; else PathRadiusOffset = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathTaperX() const { return (double)PathTaperX; }
    void setPathTaperX(double value) { if(value>1) PathTaperX=1; else if(value<-1) PathTaperX=-1; else PathTaperX = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathTaperY() const { return (double)PathTaperY; }
    void setPathTaperY(double value) { if(value>1) PathTaperY=1; else if(value<-1) PathTaperY=-1; else PathTaperY = (float)value; }

    // 0 to 3, quanta = 0.015 NOT EDITABLE IN PROP EDITOR, caused too much problems (corrupted prims with wrong values)
    double getPathRevolutions() const { return (double)PathRevolutions; }
    void setPathRevolutions(double value) { if(value>3) PathRevolutions=3; else if(value<0) PathRevolutions=0; else PathRevolutions = (float)value; }

    // -1 to 1, quanta = 0.01
    double getPathSkew() const { return (double)PathSkew; }
    void setPathSkew(double value) { if(value>1) PathSkew=1; else if(value<-1) PathSkew=-1; else PathSkew = (float)value; }
    
    // 0 to 1, quanta = 0.01
    double getProfileBegin() const { return (double)ProfileBegin; }
    void setProfileBegin(double value) { if(value>1) ProfileBegin=1; else if(value<0) ProfileBegin=0; else ProfileBegin = (float)value; }

    // 0 to 1, quanta = 0.01
    double getProfileEnd() const { return (double)ProfileEnd; }
    void setProfileEnd(double value) { if(value>1) ProfileEnd=1; else if(value<0) ProfileEnd=0; else ProfileEnd = (float)value; }

    // 0 to 1, quanta = 0.01
    double getProfileHollow() const { return (double)ProfileHollow; }
    void setProfileHollow(double value) {if(value>1) ProfileHollow=1; else if(value<0) ProfileHollow=0; else ProfileHollow = (float)value; }

    bool getHasPrimShapeData() const { return HasPrimShapeData; }

    QString getFullId() const { return QString(FullId.ToString().c_str()); }

    bool getLightCreatesShadows() const { return LightCreatesShadows; }
    void setLightCreatesShadows(bool value) { LightCreatesShadows = value; }

    bool getDescriptionTexture() const { return DescriptionTexture; }
    void setDescriptionTexture(bool value) { DescriptionTexture = value; }

    bool getIsVisible() const { return IsVisible; }
    void setIsVisible(bool value) { IsVisible = value; }

    bool getScaleToPrim() const { return ScaleToPrim; }
    void setScaleToPrim(bool value) { ScaleToPrim = value; }

    double getAnimationRate() const { return (double)AnimationRate; }
    void setAnimationRate(double value) { AnimationRate = (float)value; }

    double getDrawDistance() const { return (double)DrawDistance; }
    void setDrawDistance(double value) { DrawDistance = (float)value; }

    double getLOD() const { return (double)LOD; }
    void setLOD(double value) { LOD = (float)value; }

    void setSelectPriority(const QVariant value) { SelectPriority = value.toULongLong(); }
    QVariant getSelectPriority() const { return QVariant(static_cast<qulonglong>(SelectPriority)); }

    void setParentId(unsigned int value) { ParentId = value; } ///\todo send the update packet upstream
    unsigned int getParentId() const { return ParentId; }

    void setUpdateFlags(unsigned int value) { UpdateFlags = value; }
    unsigned int getUpdateFlags() const { return UpdateFlags; }

    QVariantMap getMaterials();
    void setMaterials(QVariantMap &qvmap);

#ifdef _DEBUG
    void PrintDebug();
#endif

    ///\todo Make the variables private
    // !ID related
    uint64_t RegionHandle;
    uint32_t LocalId;
    RexUUID FullId;
    uint32_t ParentId;
    uint8_t State;
    uint32_t CRC;
    uint16_t TimeDilation;
    uint8_t PCode;
    std::string ObjectName;

    std::string Description;
    std::string HoveringText;
    std::string MediaUrl;

    uint8_t Material;
    uint8_t ClickAction;
    uint32_t UpdateFlags;

    std::string ServerScriptClass;

    RexTypes::RexAssetID CollisionMeshID;

    RexTypes::RexAssetID SoundID;
    float SoundVolume;
    float SoundRadius;

    uint8_t *TextureAnimBuffer; //this is here as a buffer, since we don't use the TextureAnim yet
    uint8_t *NameValueBuffer; //this is here as a buffer, since we don't use the NameValue yet
    uint8_t *DataBuffer; //this is here as a buffer, since we don't use the Data yet
    uint8_t *TextColor;

    int32_t SelectPriority;

    RexUUID Sound;
    RexUUID OwnerID;

    float Gain;
    float Radius;

    uint8_t JointType;
    uint8_t Flags;

    Vector3df JointPivot;
    Vector3df JointAxisOrAnchor;

    //! Drawing related variables
    Vector3df Scale; //not a property
    uint8_t DrawType; //enum!
    bool IsVisible; 
    bool CastShadows; 
    bool LightCreatesShadows;
    bool DescriptionTexture;
    bool ScaleToPrim;
    float DrawDistance;
    float LOD;

    RexTypes::RexAssetID MeshID;
    RexTypes::RexAssetID ParticleScriptID; 

    //! Animation
    RexTypes::RexAssetID AnimationPackageID;
    std::string AnimationName; 
    float AnimationRate;

    //! reX materials
    MaterialMap Materials; //not a property

    //! Primitive texture entry data
    RexTypes::RexAssetID PrimDefaultTextureID;
    TextureMap PrimTextures; //not a property

    Color PrimDefaultColor; //not a property
    ColorMap PrimColors; //not a property

    uint8_t PrimDefaultMaterialType;
    MaterialTypeMap PrimMaterialTypes; //not a property

    Real PrimDefaultRepeatU;
    Real PrimDefaultRepeatV;
    Real PrimDefaultOffsetU;
    Real PrimDefaultOffsetV;
    Real PrimDefaultUVRotation;
    UVParamMap PrimRepeatU;
    UVParamMap PrimRepeatV;
    UVParamMap PrimOffsetU;
    UVParamMap PrimOffsetV;
    UVParamMap PrimUVRotation;

    //! Primitive shape related variables
    uint8_t PathCurve; 
    uint8_t ProfileCurve;

    float PathBegin;
    float PathEnd;
    float PathScaleX;
    float PathScaleY;
    float PathShearX;
    float PathShearY;
    float PathTwist;
    float PathTwistBegin;
    float PathRadiusOffset;
    float PathTaperX;
    float PathTaperY;
    float PathRevolutions;
    float PathSkew;
    float ProfileBegin;
    float ProfileEnd;
    float ProfileHollow;
    bool HasPrimShapeData;

public slots:
    QStringList GetChildren();

    void SetEditor(QObject *editor);
    void MyPropertyChanged(QObject * obj, const QString & property_name, const QVariant & old_value, const QVariant & new_value);
    void SendRexPrimDataUpdate();
    void SendObjectShapeUpdate();

private:
    EC_OpenSimPrim(Foundation::ModuleInterface* module);

    QObject *editor_;
    QTimer *rex_prim_data_timer_;
    QTimer *object_shape_update_timer_;
    bool rex_property_changes_;
    bool object_shape_property_changes_;

    QStringList rex_prim_data_properties_;
    QStringList object_shape_update_properties_;

signals:
    void RexPrimDataChanged(Scene::Entity*);
    void PrimShapeChanged(const EC_OpenSimPrim&);

};

#endif
