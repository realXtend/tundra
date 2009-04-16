// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimAvatar_h
#define incl_EC_OpenSimAvatar_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    class EC_OpenSimAvatar : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OpenSimAvatar);
       
    public:
        virtual ~EC_OpenSimAvatar();
     
        // !ID related
        uint64_t RegionHandle;
        uint32_t LocalId;
        RexTypes::RexUUID FullId;
        uint32_t ParentId;
        
        std::string FirstName;
        std::string LastName;

    private:
        EC_OpenSimAvatar(Foundation::ModuleInterface* module);

    };
}

#endif