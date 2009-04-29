// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimPrim_h
#define incl_EC_OpenSimPrim_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    //! Material data structure
    struct MaterialData
    {
        uint8_t Type;
        RexTypes::RexUUID UUID;
    }; 
    // Map for holding materials
    typedef std::map<uint8_t, MaterialData> MaterialMap;

    //! Each scene entity representing a prim in OpenSim sense has
    //! this component.
    class EC_OpenSimPrim : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OpenSimPrim);
    public:
        virtual ~EC_OpenSimPrim();

        // !ID related
        uint64_t RegionHandle;
        uint32_t LocalId;
        RexTypes::RexUUID FullId;
        uint32_t ParentId; 
        
        std::string ObjectName;
        std::string Description;
        std::string HoveringText;
        std::string MediaUrl;

        uint8_t Material;
        uint8_t ClickAction;
        uint32_t UpdateFlags;        

        std::string ServerScriptClass;
        
        RexTypes::RexUUID CollisionMesh;
        
        RexTypes::RexUUID SoundUUID;
        float SoundVolume;
        float SoundRadius;
        
        uint32_t SelectPriority;

        //! Drawing related variables
        uint8_t DrawType;
        bool IsVisible;
        bool CastShadows;
        bool LightCreatesShadows;
        bool DescriptionTexture;
        bool ScaleToPrim;
        float DrawDistance;
        float LOD;
        
        RexTypes::RexUUID MeshUUID;
        RexTypes::RexUUID ParticleScriptUUID;
        
        //! Animation
        RexTypes::RexUUID AnimationPackageUUID;
        std::string AnimationName;
        float AnimationRate;

        MaterialMap Materials;

        void PrintDebug();
        
    private:
        EC_OpenSimPrim(Foundation::ModuleInterface* module);
    };
}

#endif
