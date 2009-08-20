// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_LegacyAvatarSerializer_h
#define incl_RexLogic_LegacyAvatarSerializer_h

#include "EC_AvatarAppearance.h"

class QDomDocument;
class QDomElement;

namespace RexLogic
{
    //! Utility functions for dealing with reX legacy avatar definitions (xml data). Used by RexLogicModule::AvatarAppearance.
    class LegacyAvatarSerializer
    {
    public:
        //! Reads avatar definition into an EC_AvatarAppearance from an xml document
        //! \return true if mostly successful
        static bool ReadAvatarAppearance(EC_AvatarAppearance& dest, const QDomDocument& source);
        
        //! Reads animation definitions only from an xml fdocument
        //! \return true if successful
        static bool ReadAnimationDefinitions(AnimationDefinitionMap& dest, const QDomDocument& source);
        
        //! Reads a single animation definition from an xml node
        //! \return true if successful
        static bool ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem);
    };
}

#endif
