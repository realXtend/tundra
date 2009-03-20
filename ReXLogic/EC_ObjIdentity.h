// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ObjIdentity_h
#define incl_EC_ObjIdentity_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class EC_ObjIdentity : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_ObjIdentity);
    public:
        virtual ~EC_ObjIdentity();

        unsigned long long int RegionHandle;
        unsigned long Id;
        std::string FullId; // tucofixme, change type to rexuuid?
        std::string OwnerId; // tucofixme, change type to rexuuid?
        std::string ParentId; // tucofixme, change type to rexuuid?
        
        std::string ObjectName;
        std::string Description;
        
        void HandleObjectUpdate(Foundation::EventDataInterface* data);
    private:
        EC_ObjIdentity(Foundation::ModuleInterface* module);

    };
}

#endif
