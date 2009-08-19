// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LegacyAvatarSerializer.h"
#include "AnimationDefinition.h"
#include "RexLogicModule.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>

namespace RexLogic
{
    void LegacyAvatarSerializer::ReadAnimationDefinitions(AnimationDefinitionMap& dest, const std::string& filename)
    {
        PROFILE(Avatar_ReadAnimationDefinitions);
        
        dest.clear();
        
        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            RexLogicModule::LogError("Could not open avatar animations file " + filename);
            return;
        }
        QDomDocument doc("animations");
        if (!doc.setContent(&file))
        {
            file.close();
            RexLogicModule::LogError("Could not load avatar animations file " + filename);
            return;
        }
        file.close();
        
        QDomElement elem = doc.firstChildElement("animations");
        if (elem.isNull())
        {
            RexLogicModule::LogError("No avatar animation definitions in file " + filename);
            return;
        }
        
        elem = elem.firstChildElement();
        while (!elem.isNull())
        {
            ReadAnimationDefinition(dest, elem);
            elem = elem.nextSiblingElement();
        }
    }
    
    void LegacyAvatarSerializer::ReadAnimationDefinition(AnimationDefinitionMap& dest, const QDomElement& elem)
    {
        if (elem.tagName() != "animation")
            return;
        
        try
        {
            std::string id = elem.attribute("id").toStdString();
            if (id.empty())
                id = elem.attribute("uuid").toStdString(); // legacy
            if (id.empty())
            {
                RexLogicModule::LogError("Missing animation identifier");
                return; // Can't identify animation
            }
            
            std::string intname = elem.attribute("internal_name").toStdString();
            if (intname.empty())
                intname = elem.attribute("ogrename").toStdString(); // legacy
            if (intname.empty())
            {
                RexLogicModule::LogError("Missing mesh animation name");
                return; // Can't map animation to mesh animation
            }
            
            AnimationDefinition new_def;
            new_def.id_ = id;
            new_def.animation_name_ = intname;
            new_def.name_ = elem.attribute("name").toStdString();
            new_def.looped_ = Core::ParseString<bool>(elem.attribute("looped", "0").toStdString());
            new_def.exclusive_ = Core::ParseString<bool>(elem.attribute("exclusive", "0").toStdString());
            new_def.use_velocity_ = Core::ParseString<bool>(elem.attribute("usevelocity", "0").toStdString());
            new_def.always_restart_ = Core::ParseString<bool>(elem.attribute("alwaysrestart", "0").toStdString());
            new_def.fadein_ = Core::ParseString<Core::Real>(elem.attribute("fadein", "0").toStdString());
            new_def.fadeout_ = Core::ParseString<Core::Real>(elem.attribute("fadeout", "0").toStdString());
            new_def.speedfactor_ = Core::ParseString<Core::Real>(elem.attribute("speedfactor", "1").toStdString());
            new_def.weightfactor_ = Core::ParseString<Core::Real>(elem.attribute("weightfactor", "1").toStdString());
            
            dest[RexUUID(new_def.id_)] = new_def;
        }
        catch(boost::bad_lexical_cast)
        {
            RexLogicModule::LogError("Malformed animation definition");
        }
    }
}
