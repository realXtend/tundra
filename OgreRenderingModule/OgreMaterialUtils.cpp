// For conditions of distribution and use, see copyright notice in license.txt
/// @file OgreMaterialUtils.cpp
/// Contains some common methods for 

#include "StableHeaders.h"

#include "OgreMaterialUtils.h"

namespace OgreRenderer
{
    Ogre::MaterialPtr GetOrCreateLitTexturedMaterial(const char *materialName)
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

    Ogre::MaterialPtr GetOrCreateUnlitTexturedMaterial(const char *materialName)
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
    
    void SetTextureUnitOnMaterial(Ogre::MaterialPtr material, const char *textureName, Core::uint index)
    {
        Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr tex = tm.getByName(textureName);
        bool has_alpha = false;
        if (!tex.isNull())
        {
            if (Ogre::PixelUtil::hasAlpha(tex->getFormat()))
                has_alpha = true;
        }
        
        Ogre::Material::TechniqueIterator iter = material->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                // Crude alpha reject on/off based on whether texture has alpha
                //if (has_alpha)
                //    pass->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, 128);
                //else
                //    pass->setAlphaRejectFunction(Ogre::CMPF_ALWAYS_PASS);
                    
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                Core::uint cmp_index = 0;
                
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    if ((index == cmp_index) || (index == SET_ALL_UNITS))
                    {
                        if (!tex.isNull())
                            texUnit->setTextureName(textureName);
                        else
                            texUnit->setTextureName("TextureMissing.png");
                    }
                    cmp_index++;
                }
            }
        }
    }
}
