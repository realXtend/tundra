// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ObjRights_h
#define incl_EC_ObjRights_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class EC_ObjRights : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_ObjRights);
       
    public:
        virtual ~EC_ObjRights();

    private:
        EC_ObjRights(Foundation::ModuleInterface* module);

    };
}

#endif