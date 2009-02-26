#ifndef __incl_EC_SelectPriority_h__
#define __incl_EC_SelectPriority_h__

#include "ComponentInterface.h"
#include "Foundation.h"


class EC_SelectPriority : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_SelectPriority);
public:
   virtual ~EC_SelectPriority();

    static std::vector<std::string> getNetworkMessages()
    {
        std::vector<std::string> myinterest;
        myinterest.push_back("GeneralMessage_ExtraEntityData");
        return myinterest;
    } 

private:
    EC_SelectPriority();

    int mSelectPriority;
};

#endif