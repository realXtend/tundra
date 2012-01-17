/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   OgreMaterialUtils.cpp
    @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreMaterialUtils.h"
#include "OgreRenderingModule.h"
#include "AssetAPI.h"
#include "QtUtils.h"

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

QSet<QString> ProcessMaterialFileForTextures(const QString& matfilename, const QSet<QString>& used_materials)
{
    QSet<QString> used_textures;
    
    bool known_material = false;
    
    // Read the material file
    QFile matfile(matfilename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + matfilename);
        return used_textures;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();

        if (bytes.size())
        {
            int num_materials = 0;
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
#include "DisableMemoryLeakCheck.h"
            Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
            
            while(!data->eof())
            {
                std::string line = data->getLine();
                
                // Skip empty lines & comments
                if ((line.length()) && (line.substr(0, 2) != "//"))
                {
                    // Process opening/closing braces
                    if (!OgreRenderer::ProcessBraces(line, brace_level))
                    {
                        // If not a brace and on level 0, it should be a new material
                        if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                        {
                            std::string matname = line.substr(9);
                            ReplaceCharInplace(matname, '/', '_');
                            line = "material " + matname;
                            if (used_materials.find(matname.c_str()) == used_materials.end())
                            {
                                known_material = false;
                            }
                            else
                            {
                                known_material = true;
                                ++num_materials;
                            }
                        }
                        else
                        {
                            // Check for textures
                            if (known_material)
                                if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                                    used_textures.insert(line.substr(8).c_str());
                        }
                    }
                    else
                    {
                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                }
            }
        }
    }
    
    return used_textures;
}

QSet<QString> ProcessMaterialForTextures(const QString &material)
{
    QSet<QString> textures;
    QStringList lines = material.split("\n");
    for(int i = 0; i < lines.size(); ++i)
    {
        int idx = lines[i].indexOf("texture ");
        if (idx != -1)
            textures.insert(lines[i].mid(idx + 8).trimmed());
    }

    return textures;
}

QString LoadSingleMaterialFromFile(const QString &filename, const QString &materialName)
{
    QString material;

    bool right_material = false;

    // Read the material file
    QFile matfile(filename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + filename);
        return material;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();
        if (bytes.size() == 0)
        {
            LogError("Empty material file: " + filename);
            return material;
        }

        int brace_level = 0;
        bool skip_until_next = false;
        int skip_brace_level = 0;

#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
        while(!data->eof())
        {
            std::string line = data->getLine();

            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Process opening/closing braces
                if (!OgreRenderer::ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new material
                    if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                    {
                        std::string matname = line.substr(9);
                        ReplaceCharInplace(matname, '/', '_');
                        line = "material " + matname;
                        if (matname.c_str() == materialName)
                            right_material = true;
                        else
                            right_material = false;
                    }

                    // Write line to the modified copy
                    if (!skip_until_next && right_material)
                    {
                        // Add indentation.
                        for(int i =0; i < brace_level; ++i)
                            material.append("    ");

                        material.append(line.c_str());
                        material.append("\n");
                    }
                }
                else
                {
                    // Write line to the modified copy
                    if (!skip_until_next && right_material)
                    {
                        // Add indentation.
                        int i = 0;
                        if (line.find("{") != std::string::npos)
                            ++i;
                        for(; i < brace_level; ++i)
                            material.append("    ");

                        material.append(line.c_str());
                        material.append("\n");
                    }

                    if (brace_level <= skip_brace_level)
                    {
                        skip_until_next = false;
                        ///\todo return material; here?
                    }
                }
            }
        }
    }

    return material;
}

std::set<MaterialInfo> LoadAllMaterialsFromFile(const QString &filename)
{
    std::set<MaterialInfo> materials;

    // Read the material file
    QFile matfile(filename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + filename);
        return materials;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();
        if (bytes.size() == 0)
        {
            LogError("Empty material file: " + filename);
            return materials;
        }

        int brace_level = 0;
        int skip_brace_level = 0;

        MaterialInfo material;
        material.source = filename;

#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
        while(!data->eof())
        {
            std::string line = data->getLine();

            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Process opening/closing braces
                if (!OgreRenderer::ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new material
                    if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                    {
                        QString matname = line.substr(9).c_str();
                        matname.replace('/', '_');
                        material.name = matname.trimmed();
                        material.data.clear();
                    }

                    // Add indentation.
                    for(int i =0; i < brace_level; ++i)
                        material.data.append("    ");

                    material.data.append(line.c_str());
                    material.data.append("\n");
                }
                else
                {
                    // Add indentation.
                    int i = 0;
                    if (line.find("{") != std::string::npos)
                        ++i;
                    for(; i < brace_level; ++i)
                        material.data.append("    ");

                    material.data.append(line.c_str());
                    material.data.append("\n");

                    if (brace_level <= skip_brace_level)
                        materials.insert(material);
                }
            }
        }
    }

    return materials;
}

QStringList FindMaterialFiles(const QString &dir)
{
    QStringList files;
    foreach(const QString &file, DirectorySearch(dir, true, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks))
        if (file.endsWith(".material", Qt::CaseInsensitive))
            files.append(file);

    return files;
}

}
