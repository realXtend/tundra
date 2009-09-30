// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LegacyAvatarSerializer.h"
#include "RexLogicModule.h"
#include "RexTypes.h"
#include "RexNetworkUtils.h"
#include "OgreConversionUtils.h"
#include "OgreImageTextureResource.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"
#include "OgreSkeletonResource.h"

#include "HttpUtilities.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

#include <Ogre.h>

namespace RexLogic
{
    std::string modifier_mode[] = {
        "relative",
        "absolute",
        "cumulative"
    };
    
    Core::Vector3df ParseVector3(const std::string& text)
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
    
    Core::Quaternion ParseQuaternion(const std::string& text)
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
    
    Core::Quaternion ParseEulerAngles(const std::string& text)
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
    
    std::string WriteBool(bool value)
    {
        if (value)
            return "true";
        else
            return "false";
    }
    
    std::string WriteVector3(const Core::Vector3df& vector)
    {
        return Core::ToString<Core::Real>(vector.x) + " " +
            Core::ToString<Core::Real>(vector.y) + " " +
            Core::ToString<Core::Real>(vector.z);
    }
    
    std::string WriteQuaternion(const Core::Quaternion& quat)
    {
        return Core::ToString<Core::Real>(quat.w) + " " +
            Core::ToString<Core::Real>(quat.x) + " " +
            Core::ToString<Core::Real>(quat.y) + " " +
            Core::ToString<Core::Real>(quat.z);
    }
    
    std::string WriteEulerAngles(const Core::Quaternion& quat)
    {
        Core::Vector3df radians;
        quat.toEuler(radians);
        
        return Core::ToString<Core::Real>(radians.x * Core::RADTODEG) + " " +
            Core::ToString<Core::Real>(radians.y * Core::RADTODEG) + " " + 
            Core::ToString<Core::Real>(radians.z * Core::RADTODEG);
        
        //Ogre::Matrix3 rotMatrix;
        //Ogre::Quaternion value = OgreRenderer::ToOgreQuaternion(quat);
        //value.ToRotationMatrix(rotMatrix);
        //Ogre::Radian anglex;
        //Ogre::Radian angley;
        //Ogre::Radian anglez;
        //rotMatrix.ToEulerAnglesXYZ(anglex, angley, anglez);

        //Core::Real angles[3];
        //angles[0] = anglex.valueDegrees();
        //angles[1] = angley.valueDegrees();
        //angles[2] = anglez.valueDegrees();
        //
        //return Core::ToString<Core::Real>(angles[0]) + " " +
        //    Core::ToString<Core::Real>(angles[1]) + " " + 
        //    Core::ToString<Core::Real>(angles[2]);
    }
    
    void SetAttribute(QDomElement& elem, const std::string& name, const std::string& value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(value));
    }
    
    
    void SetAttribute(QDomElement& elem, const std::string& name, Core::Real value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(Core::ToString<Core::Real>(value)));
    }
    
    void SetAttribute(QDomElement& elem, const std::string& name, bool value)
    {
        elem.setAttribute(QString::fromStdString(name), QString::fromStdString(WriteBool(value)));
    }
    
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
        QDomElement base_elem = avatar.firstChildElement("base");
        if (!base_elem.isNull())
        {
            AvatarAsset mesh;
            mesh.name_ = base_elem.attribute("mesh").toStdString();
            dest.SetMesh(mesh);
        }
        
        // Get skeleton
        QDomElement skeleton_elem = avatar.firstChildElement("skeleton");
        if (!skeleton_elem.isNull())
        {
            AvatarAsset skeleton;
            skeleton.name_ = skeleton_elem.attribute("name").toStdString();
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
    
    void LegacyAvatarSerializer::WriteAvatarAppearance(QDomDocument& dest, const EC_AvatarAppearance& source)
    {
        // Version element
        {
            QDomElement version = dest.createElement("version");
            QDomText text = dest.createTextNode("0.2");
            version.appendChild(text);
            dest.appendChild(version);
        }
        
        // Avatar element
        QDomElement avatar = dest.createElement("avatar");
        
        // Mesh element
        {
            QDomElement mesh = dest.createElement("base");
            mesh.setAttribute("name", "default");
            SetAttribute(mesh, "mesh", source.GetMesh().name_);
            avatar.appendChild(mesh);
        }
        
        // Skeleton element
        {
            QDomElement skeleton = dest.createElement("skeleton");
            SetAttribute(skeleton, "name", source.GetSkeleton().name_);
            avatar.appendChild(skeleton);
        }
        
        // Material elements
        const AvatarMaterialVector& materials = source.GetMaterials();
        for (Core::uint i = 0; i < materials.size(); ++i)
        {
            // Append elements in submesh order
            QDomElement material = dest.createElement("material");
            
            std::string mat_name = materials[i].asset_.name_;
            // Strip away the file extension if exists
            std::size_t idx = (mat_name.find(".material"));
            if (idx != std::string::npos)
                mat_name = mat_name.substr(0, idx);
            
            SetAttribute(material, "name", mat_name);
            
            avatar.appendChild(material);
        }
        
        // Texture override elements
        // Face
        if (materials.size() >= 2)
        {
            if (materials[1].textures_.size() && (!materials[1].textures_[0].name_.empty()))
            {
                QDomElement texture_face = dest.createElement("texture_face");
                SetAttribute(texture_face, "name", materials[1].textures_[0].name_);
                avatar.appendChild(texture_face);
            }
        }
        // Body
        if (materials.size() >= 1)
        {
            if (materials[0].textures_.size() && (!materials[0].textures_[0].name_.empty()))
            {
                QDomElement texture_body = dest.createElement("texture_body");
                SetAttribute(texture_body, "name", materials[0].textures_[0].name_);
                avatar.appendChild(texture_body);
            }
        }
        
        // Transformation element
        {
            QDomElement transformation = dest.createElement("transformation");
            const Transform& transform  = source.GetTransform();
            SetAttribute(transformation, "position", WriteVector3(transform.position_));
            SetAttribute(transformation, "rotation", WriteQuaternion(transform.orientation_));
            SetAttribute(transformation, "scale", WriteVector3(transform.scale_));
            avatar.appendChild(transformation);
        }
        
        // Bone modifiers
        const BoneModifierSetVector& bone_modifiers = source.GetBoneModifiers();
        for (Core::uint i = 0; i < bone_modifiers.size(); ++i)
        {
            WriteBoneModifierSet(dest, avatar, bone_modifiers[i]);
        }
        
        // Morph modifiers
        const MorphModifierVector& morph_modifiers = source.GetMorphModifiers();
        for (Core::uint i = 0; i < morph_modifiers.size(); ++i)
        {
            QDomElement morph = WriteMorphModifier(dest, morph_modifiers[i]);
            avatar.appendChild(morph);
        }
        
        // Animations
        const AnimationDefinitionMap& animations = source.GetAnimations();
        AnimationDefinitionMap::const_iterator i = animations.begin();
        while (i != animations.end())
        {
            QDomElement anim = WriteAnimationDefinition(dest, i->second);
            avatar.appendChild(anim);
            ++i;
        }
        
        dest.appendChild(avatar);
    }
    
    QDomElement LegacyAvatarSerializer::WriteAnimationDefinition(QDomDocument& dest, const AnimationDefinition& anim)
    {
        QDomElement elem = dest.createElement("animation");
        
        SetAttribute(elem, "name", anim.name_);
        SetAttribute(elem, "id", anim.id_);
        SetAttribute(elem, "internal_name", anim.animation_name_);
        SetAttribute(elem, "looped", anim.looped_);
        SetAttribute(elem, "usevelocity", anim.use_velocity_);
        SetAttribute(elem, "alwaysrestart", anim.always_restart_);
        SetAttribute(elem, "fadein", anim.fadein_);
        SetAttribute(elem, "fadeout", anim.fadeout_);
        SetAttribute(elem, "speedfactor", anim.speedfactor_);
        SetAttribute(elem, "weightfactor", anim.weightfactor_);
        
        return elem;
    }
    
    void LegacyAvatarSerializer::WriteBoneModifierSet(QDomDocument& dest, QDomElement& dest_elem, const BoneModifierSet& bones)
    {
        QDomElement parameter = dest.createElement("dynamic_animation_parameter");
        QDomElement modifier = dest.createElement("dynamic_animation");
        
        SetAttribute(parameter, "name", bones.name_);
        SetAttribute(parameter, "position", bones.value_);
        SetAttribute(modifier, "name", bones.name_);

        QDomElement base_animations = dest.createElement("base_animations");
        modifier.appendChild(base_animations);
        
        QDomElement bonelist = dest.createElement("bones");
        for (Core::uint i = 0; i < bones.modifiers_.size(); ++i)
        {
            QDomElement bone = WriteBone(dest, bones.modifiers_[i]);
            bonelist.appendChild(bone);
        }
        modifier.appendChild(bonelist);
        
        if (!dest_elem.isNull())
        {
            dest_elem.appendChild(parameter);
            dest_elem.appendChild(modifier);
        }
        else
        {
            dest.appendChild(parameter);
            dest.appendChild(modifier);
        }
    }
    
    QDomElement LegacyAvatarSerializer::WriteBone(QDomDocument& dest, const BoneModifier& bone)
    {
        QDomElement elem = dest.createElement("bone");
        SetAttribute(elem, "name", bone.bone_name_);
        
        QDomElement rotation = dest.createElement("rotation");
        SetAttribute(rotation, "start", WriteEulerAngles(bone.start_.orientation_));
        SetAttribute(rotation, "end", WriteEulerAngles(bone.end_.orientation_));
        SetAttribute(rotation, "mode", modifier_mode[bone.orientation_mode_]);
        
        QDomElement translation = dest.createElement("translation");
        SetAttribute(translation, "start", WriteVector3(bone.start_.position_));
        SetAttribute(translation, "end", WriteVector3(bone.end_.position_));
        SetAttribute(translation, "mode", modifier_mode[bone.position_mode_]);

        QDomElement scale = dest.createElement("scale");
        SetAttribute(scale, "start", WriteVector3(bone.start_.scale_));
        SetAttribute(scale, "end", WriteVector3(bone.end_.scale_));
        
        elem.appendChild(rotation);
        elem.appendChild(translation);
        elem.appendChild(scale);

        return elem;
    }
    
    QDomElement LegacyAvatarSerializer::WriteMorphModifier(QDomDocument& dest, const MorphModifier& morph)
    {
        QDomElement elem = dest.createElement("morph_modifier");
        SetAttribute(elem, "name", morph.name_);
        SetAttribute(elem, "internal_name", morph.morph_name_);
        SetAttribute(elem, "influence", morph.value_);
        
        return elem;
    }
}

