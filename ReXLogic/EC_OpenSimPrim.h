// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimPrim_h
#define incl_EC_OpenSimPrim_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    class EC_OpenSimPrim : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OpenSimPrim);
    public:
        virtual ~EC_OpenSimPrim();

        // !ID related
        uint64_t RegionHandle;
        unsigned long LocalId;
        RexUUID FullId;
        RexUUID OwnerId; 
        RexUUID ParentId; 
        
        std::string ObjectName;
        std::string Description;

        void HandleObjectUpdate(Foundation::EventDataInterface* data);

    private:
        EC_OpenSimPrim(Foundation::ModuleInterface* module);



    };
}

#endif
