/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreMaterialUtils.cpp
 *  @brief  Contains some often needed utlitity functions when dealing with OGRE material scripts.
 */

#include "StableHeaders.h"
#include "OgreMaterialUtils.h"
#include "OgreMaterialResource.h"
#include "OgreRenderingModule.h"
#include <Ogre.h>

namespace OgreRenderer
{
    std::string BaseMaterials[] = {
        "LitTextured", // normal
        "UnlitTextured", // normal fullbright
        "LitTexturedAdd", // additive
        "UnlitTexturedAdd", // additive fullbright
        "LitTexturedSoftAlpha", // forced soft alpha
        "UnlitTexturedSoftAlpha", // forced soft alpha fullbright
        "LitTexturedVCol", // vertexcolor
        "UnlitTexturedVCol", // vertexcolor fullbright
        "LitTexturedSoftAlphaVCol", // vertexcolor forced soft alpha
        "UnlitTexturedSoftAlphaVCol" // vertexcolor forced soft alpha fullbright
    };
    
    std::string AlphaBaseMaterials[] = {
        "LitTexturedSoftAlpha", // soft alpha
        "UnlitTexturedSoftAlpha", // soft alpha fullbright
        "LitTexturedAdd", // additive
        "UnlitTexturedAdd", // additive fullbright
        "LitTexturedSoftAlpha", // forced soft alpha
        "UnlitTexturedSoftAlpha", // forced soft alpha fullbright
        "LitTexturedSoftAlphaVCol", // vertexcolor soft alpha
        "UnlitTexturedSoftAlphaVCol", // vertexcolor soft alpha fullbright
        "LitTexturedSoftAlphaVCol", // vertexcolor forced soft alpha
        "UnlitTexturedSoftAlphaVCol" // vertexcolor forced soft alpha fullbright
    };
    
    std::string MaterialSuffix[] = {
        "", // normal
        "fb", // normal fullbright
        "add", // additive
        "fbadd", // additive fullbright
        "alpha", // forced alpha
        "fbalpha", // forced alpha fullbright
        "vcol", // vertex color
        "fbvcol", // vertex color fullbright
        "vcolalpha", // vertex color alpha
        "fbvcolalpha" // vertex color alpha fullbright
    };

    bool IsMaterialSuffixValid(const std::string& suffix)
    {
        for (uint i = 0; i < MAX_MATERIAL_VARIATIONS; ++i)
        {
            if (suffix == MaterialSuffix[i])
                return true;
        }
        
        return false;
    }
    
    std::string GetMaterialSuffix(uint variation)
    {
        if (variation >= MAX_MATERIAL_VARIATIONS)
        {
            OgreRenderingModule::LogWarning("Requested suffix for non-existing material variation " + ToString<uint>(variation));
            variation = 0;
        }
        
        return MaterialSuffix[variation];
    }
    
    uint GetMaterialVariation(const std::string& suffix)
    {
        for (uint i = 0; i < MAX_MATERIAL_VARIATIONS; ++i)
            if (suffix == MaterialSuffix[i])
                return i;
        return 0;
    }

    Ogre::MaterialPtr CloneMaterial(const std::string& sourceMaterialName, const std::string &newName)
    {
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(sourceMaterialName);

        material = material->clone(newName);

        assert(material.get());
        return material;
    }

    Ogre::MaterialPtr GetOrCreateLitTexturedMaterial(const std::string& materialName)
    {
        const char baseMaterialName[] = "LitTextured";

        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);

        if (!material.get())
        {
            Ogre::MaterialPtr baseMaterial = mm.getByName(baseMaterialName);
            material = baseMaterial->clone(materialName);
        }

        assert(material.get());
        return material;
    }

    Ogre::MaterialPtr GetOrCreateUnlitTexturedMaterial(const std::string& materialName)
    {
        const char baseMaterialName[] = "UnlitTextured";

        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);

        if (!material.get())
        {
            Ogre::MaterialPtr baseMaterial = mm.getByName(baseMaterialName);
            material = baseMaterial->clone(materialName);
        }

        assert(material.get());
        return material;
    }

    Ogre::MaterialPtr GetOrCreateLegacyMaterial(const std::string& texture_name, const std::string& suffix)
    {
        uint variation = GetMaterialVariation(suffix);
        return GetOrCreateLegacyMaterial(texture_name, variation);
    }
    
    Ogre::MaterialPtr GetOrCreateLegacyMaterial(const std::string& texture_name, uint variation)
    {
        if (variation >= MAX_MATERIAL_VARIATIONS)
        {
            OgreRenderingModule::LogWarning("Requested suffix for non-existing material variation " + ToString<uint>(variation));
            variation = 0;
        }
        const std::string& suffix = MaterialSuffix[variation];
        
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        
        std::string material_name = texture_name + suffix;
        Ogre::MaterialPtr material = mm.getByName(material_name);
        
        if (!material.get())
        {
            material = mm.create(material_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            assert(material.get());
        }
        else
            return material;
        
        Ogre::TexturePtr tex = tm.getByName(texture_name);
        bool has_alpha = false;
        if (!tex.isNull())
        {
            // As far as the legacy materials are concerned, DXT1 is not alpha
            if (tex->getFormat() == Ogre::PF_DXT1)
                has_alpha = false;
            else if (Ogre::PixelUtil::hasAlpha(tex->getFormat()))
                has_alpha = true;
        }
        
        Ogre::MaterialPtr base_material;
        if (!has_alpha)
            base_material = mm.getByName(BaseMaterials[variation]);
        else
            base_material = mm.getByName(AlphaBaseMaterials[variation]);
        if (!base_material.get())
        {
            OgreRenderingModule::LogError("Could not find " + MaterialSuffix[variation] + " base material for " + texture_name);
            return Ogre::MaterialPtr();
        }
        
        base_material->copyDetailsTo(material);
        SetTextureUnitOnMaterial(material, texture_name, 0);
        
        return material;
    }
    
    void UpdateLegacyMaterials(const std::string& texture_name)
    {
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        
        Ogre::TexturePtr tex = tm.getByName(texture_name);
        bool has_alpha = false;
        if (!tex.isNull())
            if (Ogre::PixelUtil::hasAlpha(tex->getFormat()))
                has_alpha = true;

        for (uint i = 0; i < MAX_MATERIAL_VARIATIONS; ++i)
        {
            std::string material_name = texture_name + MaterialSuffix[i];
            Ogre::MaterialPtr material = mm.getByName(material_name);
            
            if (!material.get())
                continue;
            
            Ogre::MaterialPtr base_material;
            if (!has_alpha)
                base_material = mm.getByName(BaseMaterials[i]);
            else
                base_material = mm.getByName(AlphaBaseMaterials[i]);
            if (!base_material.get())
            {
                OgreRenderingModule::LogError("Could not find " + MaterialSuffix[i] + " base material for " + texture_name);
                continue;
            }
            
            base_material->copyDetailsTo(material);
            SetTextureUnitOnMaterial(material, texture_name, 0);
        }
    }

    void SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const std::string& texture_name, uint index)
    {
        if (material.isNull())
            return;
        
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr tex = tm.getByName(texture_name);
        
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
                            texUnit->setTextureName(texture_name);
                        else
                            texUnit->setTextureName("TextureMissing.png");
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
        
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr tex = tm.getByName(texture_name);
        
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
                    if (texUnit->getTextureName() == original_name)
                    {
                        if (tex.get())
                            texUnit->setTextureName(texture_name);
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
        
        while (i != textures_set.end())
        {
            textures.push_back(*i);
            ++i;
        }
    }
    
    Foundation::ResourcePtr CreateResourceFromMaterial(Ogre::MaterialPtr material)
    {
        assert(!material.isNull());
        OgreMaterialResource* res = new OgreMaterialResource(material->getName());
        res->SetMaterial(material);
        Foundation::ResourcePtr res_ptr(res);
        return res_ptr;
    }
    
    void RemoveMaterial(Ogre::MaterialPtr& material)
    {
        if (!material.isNull())
        {
            std::string material_name = material->getName();
            material.setNull();

            try
            {
                Ogre::MaterialManager::getSingleton().remove(material_name);
            }
            catch (Ogre::Exception& e)
            {
                OgreRenderingModule::LogDebug("Failed to remove Ogre material:" + std::string(e.what()));
            }
        }
    }

    void DebugCreateAmbientColorMaterial(const std::string &materialName, float r, float g, float b)
    {
        Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mm.getByName(materialName);
        if (material.get()) // The given material already exists, so no need to create it again.
            return;

        material = mm.getByName("SolidAmbient");
        if (!material.get())
            return;

        Ogre::MaterialPtr newMaterial = material->clone(materialName);
        newMaterial->setAmbient(r, g, b);
    }
}
