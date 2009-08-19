// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AnimationDefinition.h"

namespace RexLogic
{

    const AnimationDefinition& GetAnimationByName(const AnimationDefinitionMap& animations, const std::string& name)
    {
        static AnimationDefinition default_def;
        
        AnimationDefinitionMap::const_iterator def = animations.begin();
        while (def != animations.end())
        {
            if (def->second.animation_name_ == name)
                return def->second;
            ++def;
        }
        return default_def;
    }

}