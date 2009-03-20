// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_SelectPriority_h
#define incl_EC_SelectPriority_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class EC_SelectPriority : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_SelectPriority);
    public:
        virtual ~EC_SelectPriority();

        static std::vector<std::string> GetNetworkMessages()
        {
            std::vector<std::string> myinterest;
            myinterest.push_back("GeneralMessage_ExtraEntityData");
            return myinterest;
        } 

    private:
        EC_SelectPriority(Foundation::ModuleInterface* module);

        int select_priority_;
    };
}

#endif
