// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_LegacyAvatarSerializer_h
#define incl_RexLogic_LegacyAvatarSerializer_h

#include "AnimationDefinition.h"

class QDomElement;

namespace RexLogic
{
    //! Utility functions for dealing with reX legacy avatar definitions (xml data). Used by RexLogicModule::AvatarAppearance.
    class LegacyAvatarSerializer
    {
    public:
        //! Read animation definitions from an xml file
        static void ReadAnimationDefinitions(AnimationDefinitionMap& dest, const std::string& filename);
        
        //! Read a single animation definition from an xml node
        static void ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem);
    };
}

#endif
