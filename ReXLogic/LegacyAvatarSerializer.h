// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_LegacyAvatarSerializer_h
#define incl_RexLogic_LegacyAvatarSerializer_h

#include "EC_AvatarAppearance.h"

class QDomDocument;
class QDomElement;

namespace RexLogic
{
    class AvatarAsset;
    
    //! Utility functions for dealing with reX legacy avatar definitions (xml data). Used by RexLogicModule::AvatarAppearance.
    class LegacyAvatarSerializer
    {
    public:
        //! Reads avatar definition into an EC_AvatarAppearance from an xml document & avatar asset map
        //! \return true if mostly successful
        static bool ReadAvatarAppearance(EC_AvatarAppearance& dest, const QDomDocument& source, const std::string& appearance_address, const std::map<std::string, std::string>& asset_map);
        
        //! Reads animation definitions only from an xml document
        //! \return true if successful
        static bool ReadAnimationDefinitions(AnimationDefinitionMap& dest, const QDomDocument& source);
        
        //! Reads a bone modifier set from an xml node, and adds it to the vector
        //! \return true if successful
        static bool ReadBoneModifierSet(BoneModifierSetVector& dest, const QDomElement& elem);
        
        //! Reads a bone modifier parameter from an xml node
        /*! Actual modifiers should have been read before the parameters; this function expects a filled vector of modifier sets
            \return true if successful
         */
        static bool ReadBoneModifierParameter(BoneModifierSetVector& dest, const QDomElement& elem);
        
        //! Reads a morph modifier from an xml node, and adds it to the vector
        //! \return true if successful
        static bool ReadMorphModifier(MorphModifierVector& dest, const QDomElement& elem);
        
        //! Reads a single animation definition from an xml node, and adds it to the map
        //! \return true if successful
        static bool ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem);
        
        //! Reads an avatar attachment from an xml node, and adds it to the vector
        //! \return true if successful
        static bool ReadAttachment(AvatarAttachmentVector& dest, const QDomElement& elem, const std::string& appearance_address, const std::map<std::string, std::string>& asset_map);
        
        //! Reads a vector3 from a string, encoded as "x y z"
        static Core::Vector3df ParseVector3(const std::string& text);
        
        //! Reads a quaternion from a string, encoded as "w x y z"
        static Core::Quaternion ParseQuaternion(const std::string& text);
        
        //! Reads an euler angle rotation (degrees) from a string, returns as a quaternion
        static Core::Quaternion ParseEulerAngles(const std::string& text);
        
    private:
        static bool FillAssetId(AvatarAsset& dest, const std::string& appearance_address, const std::map<std::string, std::string>& asset_map);
    };
}

#endif
