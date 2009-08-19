// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarAppearance_h
#define incl_RexLogic_AvatarAppearance_h

#include "AnimationDefinition.h"

namespace RexLogic
{
    class RexLogicModule;
    
    //! Handles setting up and updating avatars' appearance. Used internally by RexLogicModule::Avatar.
    class AvatarAppearance
    {
    public:
        AvatarAppearance(RexLogicModule *rexlogicmodule);
        ~AvatarAppearance();
        
        //! Sets up an avatar entity's appearance
        void SetupAppearance(Scene::EntityPtr entity);
        
    private:
        RexLogicModule *rexlogicmodule_;
        
        //! Default animation definition map
        AnimationDefinitionMap default_animations_;
        
        //! Default avatar mesh name
        std::string default_avatar_mesh_;
    };
}

#endif
