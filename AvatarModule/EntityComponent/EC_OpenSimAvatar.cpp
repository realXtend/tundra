// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "IModule.h"
#include "EntityComponent/EC_OpenSimAvatar.h"

EC_OpenSimAvatar::EC_OpenSimAvatar(IModule* module) :
    IComponent(module->GetFramework()),
    controlflags(0), yaw(0), state_(Stand)
{
}

EC_OpenSimAvatar::~EC_OpenSimAvatar()
{
}

void EC_OpenSimAvatar::SetAppearanceAddress(const std::string &address, bool overrideappearance)
{
    if(overrideappearance)
        avatar_override_address_ = address;
    else
    {
        avatar_address_ = address;
        avatar_override_address_ = std::string();
    }
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
