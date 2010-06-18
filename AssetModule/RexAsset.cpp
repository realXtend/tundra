// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexAsset.h"

namespace Asset
{
    RexAsset::RexAsset(const std::string& asset_id, const std::string& asset_type) :
        asset_id_(asset_id),
        asset_type_(asset_type),
        age_(0.0)
    {
    }
}
