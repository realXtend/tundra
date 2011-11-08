/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreMaterialUtils.cpp
 *  @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreMaterialUtils.h"
#include "OgreRenderingModule.h"
#include "AssetAPI.h"
#include <Ogre.h>
#include "MemoryLeakCheck.h"

namespace OgreRenderer
{
    std::string AddDoubleQuotesIfNecessary(const std::string &str)
    {
        std::string ret = str;
        size_t found = ret.find(' ');
        if (found != std::string::npos)
        {
            ret.insert(0, "\"");
            ret.append("\"");
        }

        return ret;
    }

    void DesanitateAssetIds(std::string &script, const QStringList &keywords)
    {
        QStringList lines = QString(script.c_str()).split("\n");
        for(int i = 0; i < lines.size(); ++i)
        {
            QString id;
            int idx = -1, offset = -1;
            foreach(const QString &keyword, keywords)
                if (lines[i].contains(keyword))
                {
                    idx = lines[i].indexOf(keyword);
                    offset = keyword.length();
                    id = keyword;
                    break;
                }

            if (idx != -1 && offset != -1)
            {
                QString desanitatedRef = AssetAPI::DesanitateAssetRef(lines[i].mid(idx + offset).trimmed());
                lines[i] = lines[i].left(idx);
                lines[i].append(id + desanitatedRef);
            }
        }

        script = lines.join("\n").toStdString();
    }

    Ogre::MaterialPtr CloneMaterial(const std::string& sourceMaterialName, const std::string &newName)
    {
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(AssetAPI::SanitateAssetRef(sourceMaterialName));

        if (!material.get())
        {
             ::LogWarning("Failed to clone material \"" + sourceMaterialName + "\". It was not found.");
             return Ogre::MaterialPtr();
        }
        material = material->clone(AssetAPI::SanitateAssetRef(newName));
        if (!material.get())
        {
             ::LogWarning("Failed to clone material \"" + sourceMaterialName + "\" to name \"" + AssetAPI::SanitateAssetRef(newName) + "\"");
             return Ogre::MaterialPtr();
        }
        return material;
    }

    Ogre::MaterialPtr GetOrCreateLitTexturedMaterial(const std::string& materialName)
    {
        std::string baseMaterialName = "LitTextured";

        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(AssetAPI::SanitateAssetRef(materialName));

        if (!material.get())
        {
            Ogre::MaterialPtr baseMaterial = mm.getByName(AssetAPI::SanitateAssetRef(baseMaterialName));
            if (baseMaterial.isNull())
                return Ogre::MaterialPtr();
            
            material = baseMaterial->clone(AssetAPI::SanitateAssetRef(materialName));
        }

        assert(material.get());
        return material;
    }

    Ogre::MaterialPtr GetOrCreateUnlitTexturedMaterial(const std::string& materialName)
    {
        std::string baseMaterialName = "UnlitTextured";

        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(AssetAPI::SanitateAssetRef(materialName));

        if (!material.get())
        {
            Ogre::MaterialPtr baseMaterial = mm.getByName(AssetAPI::SanitateAssetRef(baseMaterialName));
            if (baseMaterial.isNull())
                return Ogre::MaterialPtr();
            material = baseMaterial->clone(AssetAPI::SanitateAssetRef(materialName));
        }

        assert(material.get());
        return material;
    }

    void SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const std::string& texture_name, uint index)
    {
        if (material.isNull())
            return;
        
        std::string sanitatedtexname = AssetAPI::SanitateAssetRef(texture_name);
        
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr tex = tm.getByName(sanitatedtexname);
        
        Ogre::Material::TechniqueIterator iter = material->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                uint cmp_index = 0;
                
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    if (index == cmp_index) 
                    {
                        if (tex.get())
                            texUnit->setTextureName(sanitatedtexname);
                        else
                            texUnit->setTextureName("TextureMissing.png");

                        return; // We found and replaced the index we wanted to - can early-out return without looping the rest of the indices for nothing.
                    }
                    cmp_index++;
                }
            }
        }
    }
    
    void ReplaceTextureOnMaterial(Ogre::MaterialPtr material, const std::string& original_name, const std::string& texture_name)
    {
        if (material.isNull())
            return;
        
        std::string sanitatedorgname = AssetAPI::SanitateAssetRef(original_name);
        std::string sanitatedtexname = AssetAPI::SanitateAssetRef(texture_name);
        
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr tex = tm.getByName(sanitatedtexname);
        
        Ogre::Material::TechniqueIterator iter = material->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    if (texUnit->getTextureName() == sanitatedorgname)
                    {
                        if (tex.get())
                            texUnit->setTextureName(sanitatedtexname);
                        else
                            texUnit->setTextureName("TextureMissing.png");
                    }
                }
            }
        }
    }
    
    void GetTextureNamesFromMaterial(Ogre::MaterialPtr material, StringVector& textures)
    {
        textures.clear();
        if (material.isNull())
            return;
        
        // Use a set internally to avoid duplicates
        std::set<std::string> textures_set;
        
        Ogre::Material::TechniqueIterator iter = material->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    const std::string& texname = texUnit->getTextureName();
                    
                    if (!texname.empty())
                        textures_set.insert(texname);
                }
            }
        }
        
        std::set<std::string>::iterator i = textures_set.begin();
        
        while(i != textures_set.end())
        {
            textures.push_back(*i);
            ++i;
        }
    }

    bool ProcessBraces(const std::string& line, int& braceLevel)
    {
        if (line == "{")
        {
            ++braceLevel;
            return true;
        } 
        else if (line == "}")
        {
            --braceLevel;
            return true;
        }
        else return false;
    }
}
