#include "StableHeaders.h"
#include "AssetDefines.h"
#include "AssetTransfer.h"
#include "RexAsset.h"

namespace Asset
{
    static const std::string ASSETTYPE_TEXTURE("Texture");
    static const std::string ASSETTYPE_UNKNOWN("Unknown");
    
    const std::string& RexAsset::GetTypeName()
    {
        switch (asset_type_)
        {
            case RexAT_Texture:
            return ASSETTYPE_TEXTURE;
        }
        
        return ASSETTYPE_UNKNOWN;
    }
    
    const std::string RexAsset::GetId()
    {
        return asset_id_.ToString();
    }
}