#include "RexTypes.h"
#include "RexUUID.h"

namespace RexTypes
{
    asset_type_t GetAssetTypeFromTypeName(const std::string& asset_type)
    {
        if (asset_type == ASSETTYPENAME_TEXTURE)
            return RexAT_Texture;
        if (asset_type == ASSETTYPENAME_MESH)
            return RexAT_Mesh;
        if (asset_type == ASSETTYPENAME_SKELETON)
            return RexAT_Skeleton;
        if (asset_type == ASSETTYPENAME_PARTICLE_SCRIPT)
            return RexAT_ParticleScript;
        if (asset_type == ASSETTYPENAME_MATERIAL_SCRIPT)
            return RexAT_MaterialScript;
        
        return -1;
    }
    
    const std::string& GetTypeNameFromAssetType(asset_type_t asset_type)
    {
        if (asset_type == RexAT_Texture)
            return ASSETTYPENAME_TEXTURE;
        if (asset_type == RexAT_Mesh)
            return ASSETTYPENAME_MESH;
        if (asset_type == RexAT_Skeleton)
            return ASSETTYPENAME_SKELETON;
        if (asset_type == RexAT_ParticleScript)
            return ASSETTYPENAME_PARTICLE_SCRIPT;
        if (asset_type == RexAT_MaterialScript)
            return ASSETTYPENAME_MATERIAL_SCRIPT;
        
        return ASSETTYPENAME_UNKNOWN;
    }

	bool IsNull(RexAssetID id)
	{
		if (id.size() == 0)
			return true;

		if (RexUUID::IsValid(id) && RexUUID(id).IsNull())
			return true;

		return false;
	}
}
