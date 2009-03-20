// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ObjCollision_h
#define incl_EC_ObjCollision_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class EC_ObjCollision : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_ObjCollision);
    public:
        virtual ~EC_ObjCollision();
        
        std::string CollisionMesh; // TODO tucofixme, UUID?
    private:
        EC_ObjCollision(Foundation::ModuleInterface* module);        
    };
}

#endif
