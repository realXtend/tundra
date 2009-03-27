// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_FreeData_h
#define incl_EC_FreeData_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class EC_FreeData : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_FreeData);
       
    public:
        virtual ~EC_FreeData();

        std::string FreeData;
    private:
        EC_FreeData(Foundation::ModuleInterface* module);

    };
}

#endif