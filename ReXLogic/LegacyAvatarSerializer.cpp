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
                    texture.id_ = RexTypes::ASSETTYPENAME_TEXTURE;
                    material.textures_.push_back(texture);
                }
            }
            
            dest.SetMaterial(mat_index, material);
            
            material_elem = material_elem.nextSiblingElement("material");
            mat_index++;
        }
        
        // Get animations
        QDomElement animation_elem = avatar.firstChildElement("animation");
        AnimationDefinitionMap animations;
        while (!animation_elem.isNull())
        {
            ReadAnimationDefinition(animations, animation_elem);
            animation_elem = animation_elem.nextSiblingElement("animation");
        }
        dest.SetAnimations(animations);
        
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
}
