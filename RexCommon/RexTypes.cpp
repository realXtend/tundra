#include "RexTypes.h"

namespace RexTypes
{   
    static const std::string ASSETTYPE_TEXTURE("Texture");
    static const std::string ASSETTYPE_MESH("Mesh");
    static const std::string ASSETTYPE_SKELETON("Skeleton");
    static const std::string ASSETTYPE_PARTICLE_SCRIPT("ParticleScript");
    static const std::string ASSETTYPE_MATERIAL_SCRIPT("MaterialScript");
    static const std::string ASSETTYPE_UNKNOWN("Unknown");
     
    int GetAssetTypeFromTypeName(const std::string& asset_type)
    {
        if (asset_type == ASSETTYPE_TEXTURE)
            return RexAT_Texture;
        if (asset_type == ASSETTYPE_MESH)
            return RexAT_Mesh;
        if (asset_type == ASSETTYPE_SKELETON)
            return RexAT_Skeleton;
        if (asset_type == ASSETTYPE_PARTICLE_SCRIPT)
            return RexAT_ParticleScript;
        if (asset_type == ASSETTYPE_MATERIAL_SCRIPT)
            return RexAT_MaterialScript;
            
        return -1;    
    }   
    
    const std::string& GetTypeNameFromAssetType(int asset_type)
    {
        if (asset_type == RexAT_Texture)
            return ASSETTYPE_TEXTURE;
        if (asset_type == RexAT_Mesh)
            return ASSETTYPE_MESH;
        if (asset_type == RexAT_Skeleton)
            return ASSETTYPE_SKELETON;
        if (asset_type == RexAT_ParticleScript)
            return ASSETTYPE_PARTICLE_SCRIPT;
        if (asset_type == RexAT_MaterialScript)
            return ASSETTYPE_MATERIAL_SCRIPT;
            
        return ASSETTYPE_UNKNOWN;    
    }  
}