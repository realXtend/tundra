// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Collision_h
#define incl_EC_Collision_h

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_Collision : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Collision);
public:
    virtual ~EC_Collision();
    
    static std::vector<std::string> GetNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("GeneralMessage_ExtraEntityData");
        return myinterest;
    }    
    

private:
    EC_Collision();

    std::string collision_mesh_;
};

#endif
