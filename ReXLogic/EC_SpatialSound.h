// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_SpatialSound_h
#define incl_EC_SpatialSound_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    class EC_SpatialSound : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_SpatialSound);
    public:
        virtual ~EC_SpatialSound();

        RexTypes::RexUUID SoundUUID;
        float Volume;
        float Radius;
    private:
        EC_SpatialSound(Foundation::ModuleInterface* module);


    };
}

#endif