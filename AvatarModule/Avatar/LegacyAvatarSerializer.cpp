// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LegacyAvatarSerializer.h"
#include "RexTypes.h"
#include "RexNetworkUtils.h"
#include "OgreConversionUtils.h"
#include "OgreImageTextureResource.h"
#include "OgreMaterialResource.h"
#include "OgreMeshResource.h"
#include "OgreSkeletonResource.h"
#include "RexNetworkUtils.h"
#include "HttpUtilities.h"
#include "XMLUtilities.h"

#include "AvatarModule.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

#include <Ogre.h>

using namespace RexTypes;

namespace Avatar
{
    std::string modifier_mode[] = {
        "relative",
        "absolute",
        "cumulative"
    };
    
    bool LegacyAvatarSerializer::ReadAvatarAppearance(EC_AvatarAppearance& dest, const QDomDocument& source, bool read_mesh)
    {
        PROFILE(Avatar_ReadAvatarAppearance);
        
        QDomElement avatar = source.firstChildElement("avatar");
        if (avatar.isNull())
        {
            AvatarModule::LogError("No avatar element");
            return false;
        }

        // Get mesh & skeleton
        if (read_mesh)
        {
            dest.Clear();
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
        }
               
        // Get materials, should be 2 of them
        uint mat_index = 0;
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
            AvatarTransform trans;
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
        
        // Get master modifiers
        QDomElement mastermodifier_elem = avatar.firstChildElement("master_modifier");
        MasterModifierVector mastermodifiers;
        while (!mastermodifier_elem.isNull())
        {
            ReadMasterModifier(mastermodifiers, mastermodifier_elem);
            mastermodifier_elem = mastermodifier_elem.nextSiblingElement("master_modifier");
        }              
        dest.SetMasterModifiers(mastermodifiers);
              
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
        
        // Get assetmap (optional, inventory based avatars only)
        QDomElement assetmap_elem = avatar.firstChildElement("assetmap");
        if (!assetmap_elem.isNull())
        {
            AvatarAssetMap new_map;
            QDomElement asset_elem = assetmap_elem.firstChildElement("asset");
            while (!asset_elem.isNull())
            {
                std::string name = asset_elem.attribute("name").toStdString();
                std::string id = asset_elem.attribute("id").toStdString();
                new_map[name] = id;
                asset_elem = asset_elem.nextSiblingElement("asset");
            }
            dest.SetAssetMap(new_map);
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
                    modifier.position_mode_ = BoneModifier::Absolute;
                if (trans_mode == "relative")
                    modifier.position_mode_ = BoneModifier::Relative;
                
                if (rot_mode == "absolute")
                    modifier.orientation_mode_ = BoneModifier::Absolute;
                if (rot_mode == "relative")
                    modifier.orientation_mode_ = BoneModifier::Relative;
                if (rot_mode == "cumulative")
                    modifier.orientation_mode_ = BoneModifier::Cumulative;
                
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
                dest[i].value_ = ParseReal(source.attribute("position", "0.5").toStdString());
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
        morph.value_ = ParseReal(source.attribute("influence", "0").toStdString());

        dest.push_back(morph);
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadMasterModifier(MasterModifierVector& dest, const QDomElement& source)
    {
        MasterModifier master;
        
        master.name_ = source.attribute("name").toStdString();
        master.category_ = source.attribute("category").toStdString();
        master.value_ = ParseReal(source.attribute("position", "0").toStdString());
        
        QDomElement target = source.firstChildElement("target_modifier");
        while (!target.isNull())
        {                
            SlaveModifier targetmodifier;
            targetmodifier.name_ = target.attribute("name").toStdString();
            
            std::string targettype = target.attribute("type").toStdString();
            std::string targetmode = target.attribute("mode").toStdString();
                     
            if (targettype == "morph")
                targetmodifier.type_ = AppearanceModifier::Morph;
            if (targettype == "bone")
                targetmodifier.type_ = AppearanceModifier::Bone;
            if (targettype == "dynamic_animation")
                targetmodifier.type_ = AppearanceModifier::Bone;
                
            QDomElement mapping = target.firstChildElement("position_mapping");
            while (!mapping.isNull())
            {
                SlaveModifier::ValueMapping new_mapping;
                new_mapping.master_ = ParseReal(mapping.attribute("master").toStdString());
                new_mapping.slave_ = ParseReal(mapping.attribute("target").toStdString());
                targetmodifier.mapping_.push_back(new_mapping);                    
                mapping = mapping.nextSiblingElement("position_mapping");
            }
            
            if (targetmode == "cumulative")
                targetmodifier.mode_ = SlaveModifier::Cumulative;
            else
                targetmodifier.mode_ = SlaveModifier::Average;
            
            master.modifiers_.push_back(targetmodifier);
            
            target = target.nextSiblingElement("target_modifier");
        }
        
        dest.push_back(master);
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadAnimationDefinitions(AnimationDefinitionMap& dest, const QDomDocument& source)
    {
        PROFILE(Avatar_ReadAnimationDefinitions);
        
        dest.clear();
        
        QDomElement elem = source.firstChildElement("avatar");
        if (elem.isNull())
        {
            AvatarModule::LogError("No avatar element");
            return false;
        }
        
        elem = elem.firstChildElement("animation");
        while (!elem.isNull())
        {
            ReadAnimationDefinition(dest, elem);
            elem = elem.nextSiblingElement("animation");
        }
        
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem)
    {
        if (elem.tagName() != "animation")
            return false;
        
        std::string id = elem.attribute("id").toStdString();
        if (id.empty())
            id = elem.attribute("uuid").toStdString(); // legacy
        if (id.empty())
        {
            AvatarModule::LogError("Missing animation identifier");
            return false;
        }
        
        std::string intname = elem.attribute("internal_name").toStdString();
        if (intname.empty())
            intname = elem.attribute("ogrename").toStdString(); // legacy
        if (intname.empty())
        {
            AvatarModule::LogError("Missing mesh animation name");
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
        new_def.fadein_ = ParseReal(elem.attribute("fadein", "0").toStdString());
        new_def.fadeout_ = ParseReal(elem.attribute("fadeout", "0").toStdString());
        new_def.speedfactor_ = ParseReal(elem.attribute("speedfactor", "1").toStdString());
        new_def.weightfactor_ = ParseReal(elem.attribute("weightfactor", "1").toStdString());
        
        dest[RexUUID(new_def.id_)] = new_def;
        
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
            AvatarModule::LogError("Attachment without name element");
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
            AvatarModule::LogError("Attachment without mesh element");
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
                uint idx = ParseInt(polygon.attribute("idx").toStdString());
                attachment.vertices_to_hide_.push_back(idx);
                polygon = polygon.nextSiblingElement("avatar_polygon");
            }
        }
        else
        {
            AvatarModule::LogError("Attachment without avatar element");
            return false;
        }
        
        dest.push_back(attachment);
        return true;
    }
    
    bool LegacyAvatarSerializer::ReadAttachment(AvatarAttachment& dest, const QDomDocument source, const EC_AvatarAppearance& appearance, const std::string& attachment_name)
    {
        QDomElement attachment_elem = source.firstChildElement("attachment");
        if (attachment_elem.isNull())
        {
            AvatarModule::LogError("Attachment without attachment element");
            return false;
        }

        dest.name_ = attachment_name;
        
        std::string meshname = appearance.GetMesh().name_;
        std::string basemeshname = appearance.GetProperty("basemesh");
        
        bool found = false;
        QDomElement avatar_elem = attachment_elem.firstChildElement("avatar");
        while (!avatar_elem.isNull())
        {
            std::string name = avatar_elem.attribute("name").toStdString();
            if ((name == meshname) || (name == basemeshname))
            {
                found = true;
                break;
            }   
            avatar_elem = avatar_elem.nextSiblingElement("avatar");
        }
        
        if (!found)
        {
            AvatarModule::LogError("No matching avatar mesh found in attachment. This attachment cannot be used for this avatar mesh");
            return false;
        }
        
        QDomElement mesh_elem = attachment_elem.firstChildElement("mesh");
        if (!mesh_elem.isNull())
        {
            dest.mesh_.name_ = mesh_elem.attribute("name").toStdString();
            dest.link_skeleton_ = ParseBool(mesh_elem.attribute("linkskeleton").toStdString());
        }
        else
        {
            AvatarModule::LogError("Attachment without mesh element");
            return false;
        }
        
        QDomElement bone = avatar_elem.firstChildElement("bone");
        if (!bone.isNull())
        {
            dest.bone_name_ = bone.attribute("name").toStdString();
            if (dest.bone_name_ == "None")
                dest.bone_name_ = std::string();
            dest.transform_.position_ = ParseVector3(bone.attribute("offset").toStdString());
            dest.transform_.orientation_ = ParseQuaternion(bone.attribute("rotation").toStdString());
            dest.transform_.scale_ = ParseVector3(bone.attribute("scale").toStdString());
        }
        
        QDomElement polygon = avatar_elem.firstChildElement("avatar_polygon");
        while (!polygon.isNull())
        {
            uint idx = ParseInt(polygon.attribute("idx").toStdString());
            dest.vertices_to_hide_.push_back(idx);
            polygon = polygon.nextSiblingElement("avatar_polygon");
        }

        QDomElement category_elem = attachment_elem.firstChildElement("category");
        if (!category_elem.isNull())
        {
            dest.category_ = category_elem.attribute("name").toStdString();
        }    
        
        return true;        
    }     
 
    void LegacyAvatarSerializer::WriteAvatarAppearance(QDomDocument& dest, const EC_AvatarAppearance& source, bool write_assetmap)
    {
        // Avatar element
        QDomElement avatar = dest.createElement("avatar");
        
        // Version element
        {
            QDomElement version = dest.createElement("version");
            QDomText text = dest.createTextNode("0.2");
            version.appendChild(text);
            avatar.appendChild(version);
        }
        
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
        for (uint i = 0; i < materials.size(); ++i)
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
        // Do not write these, they impose limitations on the materials
        //if (materials.size() >= 2)
        //{
        //    if (materials[1].textures_.size() && (!materials[1].textures_[0].name_.empty()))
        //    {
        //        QDomElement texture_face = dest.createElement("texture_face");
        //        SetAttribute(texture_face, "name", materials[1].textures_[0].name_);
        //        avatar.appendChild(texture_face);
        //    }
        //}
        //// Body
        //if (materials.size() >= 1)
        //{
        //    if (materials[0].textures_.size() && (!materials[0].textures_[0].name_.empty()))
        //    {
        //        QDomElement texture_body = dest.createElement("texture_body");
        //        SetAttribute(texture_body, "name", materials[0].textures_[0].name_);
        //        avatar.appendChild(texture_body);
        //    }
        //}
        
        // Transformation element
        {
            QDomElement transformation = dest.createElement("transformation");
            const AvatarTransform& transform  = source.GetTransform();
            SetAttribute(transformation, "position", WriteVector3(transform.position_));
            SetAttribute(transformation, "rotation", WriteQuaternion(transform.orientation_));
            SetAttribute(transformation, "scale", WriteVector3(transform.scale_));
            avatar.appendChild(transformation);
        }
        
        // Attachments
        const AvatarAttachmentVector& attachments = source.GetAttachments();
        for (uint i = 0; i < attachments.size(); ++i)
        {
            QDomElement attachment = WriteAttachment(dest, attachments[i], source.GetMesh());
            avatar.appendChild(attachment);
        }
        
        // Bone modifiers
        const BoneModifierSetVector& bone_modifiers = source.GetBoneModifiers();
        for (uint i = 0; i < bone_modifiers.size(); ++i)
        {
            WriteBoneModifierSet(dest, avatar, bone_modifiers[i]);
        }
        
        // Morph modifiers
        const MorphModifierVector& morph_modifiers = source.GetMorphModifiers();
        for (uint i = 0; i < morph_modifiers.size(); ++i)
        {
            QDomElement morph = WriteMorphModifier(dest, morph_modifiers[i]);
            avatar.appendChild(morph);
        }
        
        // Master modifiers
        const MasterModifierVector& master_modifiers = source.GetMasterModifiers();
        for (uint i = 0; i < master_modifiers.size(); ++i)
        {
            QDomElement master = WriteMasterModifier(dest, master_modifiers[i]);
            avatar.appendChild(master);
        }        
        
        // Animations
        const AnimationDefinitionMap& animations = source.GetAnimations();
        {
            AnimationDefinitionMap::const_iterator i = animations.begin();
            while (i != animations.end())
            {
                QDomElement anim = WriteAnimationDefinition(dest, i->second);
                avatar.appendChild(anim);
                ++i;
            }
        }
        
        // Properties
        const AvatarPropertyMap& properties = source.GetProperties();
        {
            AvatarPropertyMap::const_iterator i = properties.begin();
            while (i != properties.end())
            {         
                QDomElement prop = dest.createElement("property");

                SetAttribute(prop, "name", i->first);
                SetAttribute(prop, "value", i->second);
                avatar.appendChild(prop);
                ++i;
            }        
        }
        
        // Asset map
        if (write_assetmap)
        {
            const AvatarAssetMap& assets = source.GetAssetMap();
            {
                QDomElement map_elem = dest.createElement("assetmap");
                
                AvatarAssetMap::const_iterator i = assets.begin();
                while (i != assets.end())
                {         
                    QDomElement asset_elem = dest.createElement("asset");

                    SetAttribute(asset_elem, "name", i->first);
                    SetAttribute(asset_elem, "id", i->second);
                    map_elem.appendChild(asset_elem);
                    ++i;
                }        
                
                avatar.appendChild(map_elem);
            }   
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
        for (uint i = 0; i < bones.modifiers_.size(); ++i)
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
    
    QDomElement LegacyAvatarSerializer::WriteMasterModifier(QDomDocument& dest, const MasterModifier& master)
    {
        QDomElement elem = dest.createElement("master_modifier");
        SetAttribute(elem, "name", master.name_);
        SetAttribute(elem, "position", master.value_);
        SetAttribute(elem, "category", master.category_);
        for (uint i = 0; i < master.modifiers_.size(); ++i)
        {
            QDomElement target_elem = dest.createElement("target_modifier");
            SetAttribute(target_elem, "name", master.modifiers_[i].name_);
            if (master.modifiers_[i].type_ == AppearanceModifier::Morph)
                SetAttribute(target_elem, "type", "morph");
            else
                SetAttribute(target_elem, "type", "dynamic_animation");
            if (master.modifiers_[i].mode_ == SlaveModifier::Cumulative)
                SetAttribute(target_elem, "mode", "cumulative");
            else
                SetAttribute(target_elem, "mode", "average");
            for (uint j = 0; j < master.modifiers_[i].mapping_.size(); ++j)
            {
                QDomElement mapping_elem = dest.createElement("position_mapping");
                SetAttribute(mapping_elem, "master", master.modifiers_[i].mapping_[j].master_);
                SetAttribute(mapping_elem, "target", master.modifiers_[i].mapping_[j].slave_);
                target_elem.appendChild(mapping_elem);
            }            
            elem.appendChild(target_elem);            
        }
        
        return elem;
    }    
        
    QDomElement LegacyAvatarSerializer::WriteAttachment(QDomDocument& dest, const AvatarAttachment& attachment, const AvatarAsset& mesh)
    {
        QDomElement elem = dest.createElement("attachment");
        
        QDomElement name_elem = dest.createElement("name");
        SetAttribute(name_elem, "value", attachment.name_);
        elem.appendChild(name_elem);
      
        QDomElement mesh_elem = dest.createElement("mesh");
        SetAttribute(mesh_elem, "name", attachment.mesh_.name_);
        int link = 0;
        if (attachment.link_skeleton_) 
            link = 1;
        SetAttribute(mesh_elem, "linkskeleton", link);
        elem.appendChild(mesh_elem);
        
        QDomElement category_elem = dest.createElement("category");
        SetAttribute(category_elem, "name", attachment.category_);
        elem.appendChild(category_elem);
        
        QDomElement avatar_elem = dest.createElement("avatar");
        SetAttribute(avatar_elem, "name", mesh.name_);
        
        {
            std::string bonename = attachment.bone_name_;
            if (bonename.empty())
                bonename = "None";
            
            QDomElement bone_elem = dest.createElement("bone");
            SetAttribute(bone_elem, "name", bonename);
            SetAttribute(bone_elem, "offset", WriteVector3(attachment.transform_.position_));
            SetAttribute(bone_elem, "rotation", WriteQuaternion(attachment.transform_.orientation_));
            SetAttribute(bone_elem, "scale", WriteVector3(attachment.transform_.scale_));
            
            avatar_elem.appendChild(bone_elem);
            
            for (uint i = 0; i < attachment.vertices_to_hide_.size(); ++i)
            {
                QDomElement polygon_elem = dest.createElement("avatar_polygon");
                SetAttribute(polygon_elem, "idx", (int)attachment.vertices_to_hide_[i]);
                avatar_elem.appendChild(polygon_elem);
            }
        }
        elem.appendChild(avatar_elem);
        
        return elem;
    }
}

