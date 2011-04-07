// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_LegacyAvatarSerializer_h
#define incl_Avatar_LegacyAvatarSerializer_h

#include "EntityComponent/EC_AvatarAppearance.h"
#include <QPair>

class QDomDocument;
class QDomElement;

/// \todo Deprecated. Reimplemented in AvatarDescAsset
/*
namespace Avatar
{
    //! Utility functions for dealing with reX legacy avatar definitions (xml data). Used by RexLogicModule::AvatarAppearance.
    class LegacyAvatarSerializer
    {

    public:
        typedef QPair<bool, QString> SerializeResult;

        //! Reads avatar definition into an EC_AvatarAppearance from an xml document
        static bool ReadAvatarAppearance(EC_AvatarAppearance& dest, const QDomDocument& source, bool read_mesh = true);
        
        //! Reads animation definitions only from an xml document
        //! \return true if successful
        static bool ReadAnimationDefinitions(AnimationDefinitionMap& dest, const QDomDocument& source);
        
        //! Writes avatar definition from an EC_AvatarAppearance into an xml document
        static void WriteAvatarAppearance(QDomDocument& dest, const EC_AvatarAppearance& source, bool write_assetmap = false);
        
        //! Reads an attachment xml document
        static SerializeResult ReadAttachment(AvatarAttachment& dest, const QDomDocument source, const EC_AvatarAppearance& appearance, const std::string& attachment_name);
        
    private:
        //! Reads a bone modifier set from an xml node, and adds it to the vector
        //! \return true if successful
        static bool ReadBoneModifierSet(BoneModifierSetVector& dest, const QDomElement& source);
        
        //! Reads a bone modifier parameter from an xml node
        static bool ReadBoneModifierParameter(BoneModifierSetVector& dest, const QDomElement& source);
        
        //! Reads a morph modifier from an xml node, and adds it to the vector
        //! \return true if successful
        static bool ReadMorphModifier(MorphModifierVector& dest, const QDomElement& source);
        
        //! Reads a single animation definition from an xml node, and adds it to the map
        //! \return true if successful
        static bool ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& source);
        
        //! Reads a master modifier from an xml node, and adds it to the vec
        static bool ReadMasterModifier(MasterModifierVector& dest, const QDomElement& source);
         
        //! Reads an avatar attachment from an xml node, and adds it to the vector
        //! \return true if successful
        static SerializeResult ReadAttachment(AvatarAttachmentVector& dest, const QDomElement& source);
        
        //! Writes bone modifiers to xml document
        static void WriteBoneModifierSet(QDomDocument& dest, QDomElement& dest_elem, const BoneModifierSet& bones);
        
        //! Writes a single bone's parameters to an xml element and returns it
        static QDomElement WriteBone(QDomDocument& dest, const BoneModifier& bone);
        
        //! Writes a morph modifier to an xml element and returns it
        static QDomElement WriteMorphModifier(QDomDocument& dest, const MorphModifier& morph);

        //! Writes a master modifier to an xml element and returns it
        static QDomElement WriteMasterModifier(QDomDocument& dest, const MasterModifier& morph);
        
        //! Writes an animation definition to an xml element and returns it
        static QDomElement WriteAnimationDefinition(QDomDocument& dest, const AnimationDefinition& anim);
        
        //! Writes an avatar attachment to an xml element and returns it
        static QDomElement WriteAttachment(QDomDocument& dest, const AvatarAttachment& attachment, const AvatarAsset& mesh);
    };
}
*/

#endif
