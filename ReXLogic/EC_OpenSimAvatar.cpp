// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimAvatar.h"

namespace RexLogic
{
    EC_OpenSimAvatar::EC_OpenSimAvatar(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        RegionHandle = 0;
        LocalId = 0;
        FullId.SetNull();
        ParentId = 0;     
    
        FirstName = "";
        LastName = "";
        
        avatar_address_ = "";
        avatar_override_address_ = "";
    }

    EC_OpenSimAvatar::~EC_OpenSimAvatar()
    {
    }
    
    void EC_OpenSimAvatar::SetAppearanceAddress(std::string address, bool overrideappearance)
    {
        if(overrideappearance)
            avatar_override_address_ = address;
        else
            avatar_address_ = address;
                
    }
    
    std::string EC_OpenSimAvatar::GetAppearanceAddress()
    {
        if(avatar_override_address_.length() > 0)
            return avatar_override_address_;
        else
            return avatar_address_;   
    }
}