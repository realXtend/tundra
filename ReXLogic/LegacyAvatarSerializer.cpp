// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "RexTypes.h"
#include "RexNetworkUtils.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

namespace RexLogic
{
    bool LegacyAvatarSerializer::ReadAvatarAppearance(RexLogic::EC_AvatarAppearance& dest, const QDomDocument& source)
    {
        dest.Clear();
        
        QDomElement avatar = source.firstChildElement("avatar");
        if (avatar.isNull())
        {
            RexLogicModule::LogError("No avatar element");
            return false;
        }
        
        // Get mesh
        // Note: the legacy avatar xml just stores the asset human-readable names; actual asset ID's come from elsewhere
        // and we don't bother with them for now
        QDomElement base_elem = avatar.firstChildElement("base");
        if (!base_elem.isNull())
        {
            AvatarAsset mesh;
            mesh.name_ = base_elem.attribute("mesh").toStdString();
            mesh.type_ = RexTypes::ASSETTYPENAME_MESH;
            dest.SetMesh(mesh);
        }
        
        // Get skeleton
        QDomElement skeleton_elem = avatar.firstChildElement("skeleton");
        if (!skeleton_elem.isNull())
        {
            AvatarAsset skeleton;
            skeleton.name_ = skeleton_elem.attribute("name").toStdString();
            skeleton.type_ = RexTypes::ASSETTYPENAME_SKELETON;
            dest.SetSkeleton(skeleton);
        }
        
        // Get materials, should be 2 of them
        Core::uint mat_index = 0;
        QDomElement material_elem = avatar.firstChildElement("material");
        AvatarMaterialVector materials;
        while (!material_elem.isNull())
        {
            AvatarMaterial material;
            material.asset_.name_ = material_elem.attribute("name").toStdString();
            material.asset_.type_ = RexTypes::ASSETTYPENAME_MATERIAL_SCRIPT;
            
            // Check for texture override
            QDomElement texture_elem;
            switch (mat_index)
            {
            case 0:
                texture_elem = avatar.firstChildElement("texture_body");
                break;
            case 1:
                texture_elem = avatar.firstChildElement("texture_face");
                break;
            }
            
            if (!texture_elem.isNull())
            {
                std::string tex_name = texture_elem.attribute("name").toStdString();
                if (!tex_name.empty())
                {
                    AvatarAsset texture;
                    texture.name_ = tex_name;
                    texture.type_ = RexTypes::ASSETTYPENAME_TEXTURE;
                    material.textures_.push_back(texture);
                }
            }
            
            materials.push_back(material);
            
            material_elem = material_elem.nextSiblingElement("material");
            ++mat_index;
        }
        dest.SetMaterials(materials);
        
        // Get main transform
        QDomElement transform_elem = avatar.firstChildElement("transformation");
        if (!transform_elem.isNull())
        {
            Transform trans;
            trans.position_ = ParseVector3(transform_elem.attribute("position").toStdString());
            trans.orientation_ = ParseQuaternion(transform_elem.attribute("rotation").toStdString());
            trans.scale_ = ParseVector3(transform_elem.attribute("scale").toStdString());
            dest.SetTransform(trans);
        }
        
        // Get attachments
        QDomElement attachment_elem = avatar.firstChildElement("attachment");
        AvatarAttachmentVector attachments;
        while (!attachment_elem.isNull())
        {
            ReadAttachment(attachments, attachment_elem);
            attachment_elem = attachment_elem.nextSiblingElement("attachment");
        }
        dest.SetAttachments(attachments);
        
        // Get bone modifiers
        QDomElement bonemodifier_elem = avatar.firstChildElement("dynamic_animation");
        BoneModifierSetVector bonemodifiers;
        while (!bonemodifier_elem.isNull())
        {
            ReadBoneModifierSet(bonemodifiers, bonemodifier_elem);
            bonemodifier_elem = bonemodifier_elem.nextSiblingElement("dynamic_animation");
        }
        // Get bone modifier parameters
        QDomElement bonemodifierparam_elem = avatar.firstChildElement("dynamic_animation_parameter");
        while (!bonemodifierparam_elem.isNull())
        {
            ReadBoneModifierParameter(bonemodifiers, bonemodifierparam_elem);
            bonemodifierparam_elem = bonemodifierparam_elem.nextSiblingElement("dynamic_animation_parameter");
        }
        dest.SetBoneModifiers(bonemodifiers);
        
        // Get morph modifiers
        QDomElement morphmodifier_elem = avatar.firstChildElement("morph_modifier");
        MorphModifierVector morphmodifiers;
        while (!morphmodifier_elem.isNull())
        {
            ReadMorphModifier(morphmodifiers, morphmodifier_elem);
            morphmodifier_elem = morphmodifier_elem.nextSiblingElement("morph_modifier");
        }
        dest.SetMorphModifiers(morphmodifiers);
        
        // Get animations
        QDomElement animation_elem = avatar.firstChildElement("animation");
        AnimationDefinitionMap animations;
        while (!animation_elem.isNull())
        {
            ReadAnimationDefinition(animations, animation_elem);
            animation_elem = animation_elem.nextSiblingElement("animation");
        }
        dest.SetAnimations(animations);
        
        // Get properties
        QDomElement property_elem = avatar.firstChildElement("property");
        while (!property_elem.isNull())
        {
            std::string name = property_elem.attribute("name").toStdString();
            std::string value = property_elem.attribute("value").toStdString();
            if ((!name.empty()) && (!value.empty()))
                dest.SetProperty(name, value);
            
            property_elem = property_elem.nextSiblingElement("property");
        }
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadBoneModifierSet(BoneModifierSetVector& dest, const QDomElement& source)
    {
        BoneModifierSet modifier_set;
        modifier_set.name_ = source.attribute("name").toStdString();
        unsigned num_bones = 0;
        
        QDomElement bones = source.firstChildElement("bones");
        if (!bones.isNull())
        {
            QDomElement bone = bones.firstChildElement("bone");
            while (!bone.isNull())
            {
                BoneModifier modifier;
                modifier.bone_name_ = bone.attribute("name").toStdString();
                QDomElement rotation = bone.firstChildElement("rotation");
                QDomElement translation = bone.firstChildElement("translation");
                QDomElement scale = bone.firstChildElement("scale");
                
                modifier.start_.position_ = ParseVector3(translation.attribute("start").toStdString());
                modifier.start_.orientation_ = ParseEulerAngles(rotation.attribute("start").toStdString());
                modifier.start_.scale_ = ParseVector3(scale.attribute("start").toStdString());
                
                modifier.end_.position_ = ParseVector3(translation.attribute("end").toStdString());
                modifier.end_.orientation_ = ParseEulerAngles(rotation.attribute("end").toStdString());
                modifier.end_.scale_ = ParseVector3(scale.attribute("end").toStdString());
                
                std::string trans_mode = translation.attribute("mode").toStdString();
                std::string rot_mode = rotation.attribute("mode").toStdString();
                
                if (trans_mode == "absolute")
                    modifier.position_mode_ = Absolute;
                if (trans_mode == "relative")
                    modifier.position_mode_ = Relative;
                
                if (rot_mode == "absolute")
                    modifier.orientation_mode_ = Absolute;
                if (rot_mode == "relative")
                    modifier.orientation_mode_ = Relative;
                if (rot_mode == "cumulative")
                    modifier.orientation_mode_ = Cumulative;
                
                modifier_set.modifiers_.push_back(modifier);
                
                bone = bone.nextSiblingElement("bone");
                ++num_bones;
            }
        }
        
        if (num_bones)
            dest.push_back(modifier_set);
        
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadBoneModifierParameter(BoneModifierSetVector& dest, const QDomElement& source)
    {
        // Find existing modifier from the vector
        std::string name = source.attribute("name").toStdString();
        for (unsigned i = 0; i < dest.size(); ++i)
        {
            if (dest[i].name_ == name)
            {
                try
                {
                    dest[i].value_ = Core::ParseString<Core::Real>(source.attribute("position").toStdString());
                }
                catch (boost::bad_lexical_cast)
                {
                    return false;
                }
                
                return true;
            }
        }
        
        return false; // Not found
    }
    
    bool LegacyAvatarSerializer::ReadMorphModifier(MorphModifierVector& dest, const QDomElement& source)
    {
        MorphModifier morph;
        
        morph.name_ = source.attribute("name").toStdString();
        morph.morph_name_ = source.attribute("internal_name").toStdString();
        try
        {
            morph.value_ = Core::ParseString<Core::Real>(source.attribute("influence").toStdString());
        }
        catch (boost::bad_lexical_cast)
        {
            return false;
        }
        
        dest.push_back(morph);
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadAnimationDefinitions(AnimationDefinitionMap& dest, const QDomDocument& source)
    {
        PROFILE(Avatar_ReadAnimationDefinitions);
        
        dest.clear();
        
        QDomElement elem = source.firstChildElement("animations");
        if (elem.isNull())
        {
            RexLogicModule::LogError("No avatar animation definitions in xml document");
            return false;
        }
        
        bool all_success = true;
        elem = elem.firstChildElement();
        while (!elem.isNull())
        {
            all_success = all_success && ReadAnimationDefinition(dest, elem);
            elem = elem.nextSiblingElement();
        }
        
        return all_success;
    }
    
    bool LegacyAvatarSerializer::ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem)
    {
        if (elem.tagName() != "animation")
            return false;
        
        try
        {
            std::string id = elem.attribute("id").toStdString();
            if (id.empty())
                id = elem.attribute("uuid").toStdString(); // legacy
            if (id.empty())
            {
                RexLogicModule::LogError("Missing animation identifier");
                return false;
            }
            
            std::string intname = elem.attribute("internal_name").toStdString();
            if (intname.empty())
                intname = elem.attribute("ogrename").toStdString(); // legacy
            if (intname.empty())
            {
                RexLogicModule::LogError("Missing mesh animation name");
                return false;
            }
            
            AnimationDefinition new_def;
            new_def.id_ = id;
            new_def.animation_name_ = intname;
            new_def.name_ = elem.attribute("name").toStdString();
            
            new_def.looped_ = ParseBool(elem.attribute("looped", "true").toStdString());
            new_def.exclusive_ = ParseBool(elem.attribute("exclusive", "false").toStdString());
            new_def.use_velocity_ = ParseBool(elem.attribute("usevelocity", "false").toStdString());
            new_def.always_restart_ = ParseBool(elem.attribute("alwaysrestart", "false").toStdString());
            new_def.fadein_ = Core::ParseString<Core::Real>(elem.attribute("fadein", "0").toStdString());
            new_def.fadeout_ = Core::ParseString<Core::Real>(elem.attribute("fadeout", "0").toStdString());
            new_def.speedfactor_ = Core::ParseString<Core::Real>(elem.attribute("speedfactor", "1").toStdString());
            new_def.weightfactor_ = Core::ParseString<Core::Real>(elem.attribute("weightfactor", "1").toStdString());
            
            dest[RexUUID(new_def.id_)] = new_def;
        }
        catch(boost::bad_lexical_cast)
        {
            RexLogicModule::LogError("Malformed animation definition");
            return false;
        }
        
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadAttachment(AvatarAttachmentVector& dest, const QDomElement& elem)
    {
        AvatarAttachment attachment;
        
        QDomElement name = elem.firstChildElement("name");
        if (!name.isNull())
        {
            attachment.name_ = name.attribute("value").toStdString();
        }
        else
        {
            RexLogicModule::LogError("Attachment without name element");
            return false;
        }

        QDomElement category = elem.firstChildElement("category");
        if (!category.isNull())
        {
            attachment.category_ = category.attribute("name").toStdString();
        }
        
        QDomElement mesh = elem.firstChildElement("mesh");
        if (!mesh.isNull())
        {
            attachment.mesh_.name_ = mesh.attribute("name").toStdString();
            attachment.mesh_.type_ = RexTypes::ASSETTYPENAME_MESH;
            attachment.link_skeleton_ = ParseBool(mesh.attribute("linkskeleton").toStdString());
        }
        else
        {
            RexLogicModule::LogError("Attachment without mesh element");
            return false;
        }
        
        QDomElement avatar = elem.firstChildElement("avatar");
        if (!avatar.isNull())
        {
            QDomElement bone = avatar.firstChildElement("bone");
            if (!bone.isNull())
            {
                attachment.bone_name_ = bone.attribute("name").toStdString();
                if (attachment.bone_name_ == "None")
                    attachment.bone_name_ = std::string();
                attachment.transform_.position_ = ParseVector3(bone.attribute("offset").toStdString());
                attachment.transform_.orientation_ = ParseQuaternion(bone.attribute("rotation").toStdString());
                attachment.transform_.scale_ = ParseVector3(bone.attribute("scale").toStdString());
            }
            
            QDomElement polygon = avatar.firstChildElement("avatar_polygon");
            while (!polygon.isNull())
            {
                try
                {
                    Core::uint idx = Core::ParseString<Core::uint>(polygon.attribute("idx").toStdString());
                    attachment.vertices_to_hide_.push_back(idx);
                }
                catch (boost::bad_lexical_cast)
                {
                }
                polygon = polygon.nextSiblingElement("avatar_polygon");
            }
        }
        else
        {
            RexLogicModule::LogError("Attachment without avatar element");
            return false;
        }
        
        dest.push_back(attachment);
        return true;
    }
    
    Core::Vector3df LegacyAvatarSerializer::ParseVector3(const std::string& text)
    {
        Core::Vector3df vec(0.0f, 0.0f, 0.0f);
        
        Core::StringVector components = Core::SplitString(text, ' ');
        if (components.size() == 3)
        {
            try
            {
                vec.x = Core::ParseString<Core::Real>(components[0]);
                vec.y = Core::ParseString<Core::Real>(components[1]);
                vec.z = Core::ParseString<Core::Real>(components[2]);
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        return vec;
    }
    
    Core::Quaternion LegacyAvatarSerializer::ParseQuaternion(const std::string& text)
    {
        Core::Quaternion quat;
        
        Core::StringVector components = Core::SplitString(text, ' ');
        if (components.size() == 4)
        {
            try
            {
                quat.w = Core::ParseString<Core::Real>(components[0]);
                quat.x = Core::ParseString<Core::Real>(components[1]);
                quat.y = Core::ParseString<Core::Real>(components[2]);
                quat.z = Core::ParseString<Core::Real>(components[3]);
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        return quat;
    }
    
    Core::Quaternion LegacyAvatarSerializer::ParseEulerAngles(const std::string& text)
    {
        Core::Quaternion quat;
        
        Core::StringVector components = Core::SplitString(text, ' ');
        if (components.size() == 3)
        {
            try
            {
                Core::Real xrad = Core::degToRad(Core::ParseString<Core::Real>(components[0]));
                Core::Real yrad = Core::degToRad(Core::ParseString<Core::Real>(components[1]));
                Core::Real zrad = Core::degToRad(Core::ParseString<Core::Real>(components[2]));
                
                Core::Real angle = yrad * 0.5;
                double cx = cos(angle);
                double sx = sin(angle);

                angle = zrad * 0.5;
                double cy = cos(angle);
                double sy = sin(angle);

                angle = xrad * 0.5;
                double cz = cos(angle);
                double sz = sin(angle);

                quat.x = sx * sy * cz + cx * cy * sz;
                quat.y = sx * cy * cz + cx * sy * sz;
                quat.z = cx * sy * cz - sx * cy * sz;
                quat.w = cx * cy * cz - sx * sy * sz;
                
                quat.normalize();
            }
            catch (boost::bad_lexical_cast)
            {
            }
        }
        
        return quat;
    }
}
