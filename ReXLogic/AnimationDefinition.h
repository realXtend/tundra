#ifndef incl_RexLogic_AnimationDefinition_h
#define incl_RexLogic_AnimationDefinition_h

#include "RexUUID.h"

namespace RexLogic
{
    //! Defines an animation for an avatar
    struct AnimationDefinition
    {
        //! Most likely a UUID
        std::string id_;
        //! Identifying human-readable name, not mandatory and not used directly in code
        std::string name_;
        //! Actual animation name in the mesh/skeleton
        std::string animation_name_;
        //! Should play looped?
        bool looped_;
        //! Exclusive; override (stop) other animations
        bool exclusive_;
        //! Speed scaled with avatar movement speed?
        bool use_velocity_;
        //! Always restart animation when it starts playing?
        bool always_restart_;
        //! Blend-in period in seconds
        Core::Real fadein_;
        //! Blend-out period in seconds
        Core::Real fadeout_;
        //! Speed modification (1.0 original)
        Core::Real speedfactor_;
        //! Weight modification (1.0 full)
        Core::Real weightfactor_;
        
        AnimationDefinition() :
            looped_(true),
            exclusive_(false),
            use_velocity_(false),
            always_restart_(false),
            fadein_(0.0),
            fadeout_(0.0),
            speedfactor_(1.0),
            weightfactor_(1.0)
        {
        }
    };
    
    typedef std::map<RexTypes::RexUUID, AnimationDefinition> AnimationDefinitionMap;
    
    const AnimationDefinition& GetAnimationByName(const AnimationDefinitionMap& animations, const std::string& name);
}

#endif