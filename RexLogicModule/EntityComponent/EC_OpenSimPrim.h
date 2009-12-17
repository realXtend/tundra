// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimPrim_h
#define incl_EC_OpenSimPrim_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "RexUUID.h"
#include "RexTypes.h"
#include "RexLogicModuleApi.h"

namespace RexLogic
{
    //! Material data structure
    struct REXLOGIC_MODULE_API MaterialData
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

    //! Each scene entity representing a prim in OpenSim sense has
    //! this component.
    class REXLOGIC_MODULE_API EC_OpenSimPrim : public Foundation::ComponentInterface
    {
        Q_OBJECT
            
        DECLARE_EC(EC_OpenSimPrim);
    public:
        virtual ~EC_OpenSimPrim();

        // !ID related
        uint64_t RegionHandle;
        uint32_t LocalId;
        RexUUID FullId;
        uint32_t ParentId; 

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

        int32_t SelectPriority;

        //! Drawing related variables
        Vector3df Scale;
        uint8_t DrawType;
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
        MaterialMap Materials;

        //! Primitive texture entry data
        RexTypes::RexAssetID PrimDefaultTextureID;
        TextureMap PrimTextures;
        Color PrimDefaultColor;
        ColorMap PrimColors;
        uint8_t PrimDefaultMaterialType;
        MaterialTypeMap PrimMaterialTypes;

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

        void PrintDebug();

    private:
        EC_OpenSimPrim(Foundation::ModuleInterface* module);
    };
}

#endif
