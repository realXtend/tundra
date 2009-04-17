#include "StableHeaders.h"
#include "AssetDefines.h"
#include "AssetTransfer.h"
#include "RexAsset.h"

namespace Asset
{
    static const std::string ASSETTYPE_TEXTURE("Texture");
    static const std::string ASSETTYPE_MESH("Mesh");
    static const std::string ASSETTYPE_SKELETON("Skeleton");
    static const std::string ASSETTYPE_PARTICLE_SCRIPT("Particle script");
    static const std::string ASSETTYPE_MATERIAL_SCRIPT("Material script");
    static const std::string ASSETTYPE_UNKNOWN("Unknown");
    
    RexAsset::RexAsset(const RexTypes::RexUUID& asset_id, Core::asset_type_t asset_type) :
        asset_id_(asset_id),
        asset_type_(asset_type)
    {
    }
    
    const std::string& RexAsset::GetTypeName()
    {
        switch (asset_type_)
        {
            case RexAT_Texture:
            return ASSETTYPE_TEXTURE;

            case RexAT_Mesh:
            return ASSETTYPE_MESH;

            case RexAT_Skeleton:
            return ASSETTYPE_SKELETON;

            case RexAT_ParticleScript:
            return ASSETTYPE_PARTICLE_SCRIPT;

            case RexAT_MaterialScript:
            return ASSETTYPE_MATERIAL_SCRIPT;
        }
        
        return ASSETTYPE_UNKNOWN;
    }
    
    const std::string RexAsset::GetId()
    {
        return asset_id_.ToString();
    }
}