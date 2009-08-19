// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_EC_AvatarAppearance_h
#define incl_RexLogic_EC_AvatarAppearance_h

#include "ComponentInterface.h"
#include "RexTypes.h"
#include "AnimationDefinition.h"

namespace RexLogic
{
    class EC_AvatarAppearance : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_AvatarAppearance);
    public:
        virtual ~EC_AvatarAppearance();
        
        const AnimationDefinitionMap& GetAnimations() { return animations_; }
        
        void SetAnimations(const AnimationDefinitionMap& animations);
        
    private:
        EC_AvatarAppearance(Foundation::ModuleInterface* module);
        
        AnimationDefinitionMap animations_;
    };
}

#endif
