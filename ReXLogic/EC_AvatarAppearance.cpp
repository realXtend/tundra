// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_AvatarAppearance.h"

namespace RexLogic
{
    EC_AvatarAppearance::EC_AvatarAppearance(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
    }

    EC_AvatarAppearance::~EC_AvatarAppearance()
    {
    }
    
   void EC_AvatarAppearance::SetAnimations(const AnimationDefinitionMap& animations)
    {
        animations_ = animations;
    }
}
