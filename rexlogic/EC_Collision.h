// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_EC_Collision_h__
#define __incl_EC_Collision_h__

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_Collision : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Collision);
public:
    virtual ~EC_Collision();
    
    static std::vector<std::string> getNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("GeneralMessage_ExtraEntityData");
        return myinterest;
    }    
    

private:
    EC_Collision();

    std::string mCollisionMesh;
};

#endif
