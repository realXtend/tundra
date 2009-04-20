// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_TerrainPatch_h
#define incl_EC_TerrainPatch_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    /// Entities that represent a patch (16x16 vertices) of terrain have this
    /// compoonent on them.
    class EC_TerrainPatch : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_TerrainPatch);
    public:
        virtual ~EC_TerrainPatch();

        /// X coordinate on the grid of patches. In OpenSim this is [0, 15], but might change.
        int x;

        /// [0, 15].
        int y;

        /// Typically this will be a 16x16 array of height values in world coordinates.
        std::vector<float> heightData;

    private:
        EC_TerrainPatch(Foundation::ModuleInterface* module);
    };
}

#endif
