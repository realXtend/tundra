// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimAvatar.h"

namespace RexLogic
{
    EC_OpenSimAvatar::EC_OpenSimAvatar(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        state_ = Stand;
        
        //RegionHandle = 0;
        //LocalId = 0;
        //FullId.SetNull();
        //ParentId = 0;     
    
        //first_name_ = "";
        //last_name_ = "";
        
        //avatar_address_ = "";
        //avatar_override_address_ = "";
    }

    EC_OpenSimAvatar::~EC_OpenSimAvatar()
    {
    }

    //void EC_OpenSimAvatar::SetFirstName(const std::string &name)
    //{
    //    first_name_ = name;
    //} 

    //std::string EC_OpenSimAvatar::GetFirstName()
    //{
    //    return first_name_;
    //}

    //void EC_OpenSimAvatar::SetLastName(const std::string &name)
    //{
    //    last_name_ = name;
    //}

    //std::string EC_OpenSimAvatar::GetLastName()
    //{
    //    return last_name_;
    //}

    //std::string EC_OpenSimAvatar::GetFullName()
    //{
    //    return first_name_ + " " + last_name_;
    //}
    
    void EC_OpenSimAvatar::SetAppearanceAddress(const std::string &address, bool overrideappearance)
    {
        if(overrideappearance)
            avatar_override_address_ = address;
        else
            avatar_address_ = address;
    }
    
    const std::string& EC_OpenSimAvatar::GetAppearanceAddress() const
    {
        if(avatar_override_address_.length() > 0)
            return avatar_override_address_;
        else
            return avatar_address_;
    }
    
    void EC_OpenSimAvatar::SetState(State state)
    {
        state_ = state;
    }
    
    EC_OpenSimAvatar::State EC_OpenSimAvatar::GetState() const
    {
        return state_;
    }
}