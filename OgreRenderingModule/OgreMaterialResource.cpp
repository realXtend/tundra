// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreMaterialResource.h"
#include "OgreTextureResource.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialUtils.h"
#include "ResourceHandler.h"
#include "CoreStringUtils.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreMaterialResource::OgreMaterialResource(const std::string& id, ShadowQuality shadowquality) : 
        ResourceInterface(id),
        shadowquality_(shadowquality)
    {
    }

    OgreMaterialResource::OgreMaterialResource(const std::string& id, ShadowQuality shadowquality, Foundation::AssetPtr source) : 
        ResourceInterface(id),
        shadowquality_(shadowquality)
    {
        SetData(source);
    }

    OgreMaterialResource::~OgreMaterialResource()
    {
        RemoveMaterial();
    }

    void OgreMaterialResource::SetMaterial(Ogre::MaterialPtr material)
    {
        ogre_material_ = material;
    }
    
    bool OgreMaterialResource::SetData(Foundation::AssetPtr source)
    {
        // Remove old material if any
        RemoveMaterial();
        references_.clear();
        original_textures_.clear();

        Ogre::MaterialManager& matmgr = Ogre::MaterialManager::getSingleton(); 

        OgreRenderingModule::LogDebug("Parsing material " + source->GetId());
        
        if (!source)
        {
            OgreRenderingModule::LogError("Null source asset data pointer");
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized material asset");
            return false;
        }

        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(const_cast<u8 *>(source->GetData()), source->GetSize()));

        static int tempname_count = 0;
        tempname_count++;
        std::string tempname = "TempMat" + ToString<int>(tempname_count);
        
        try
        {
            int num_materials = 0;
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
            // Parsed/modified material script
            std::ostringstream output;
            

            while (!data->eof())
            {
                Ogre::String line = data->getLine();
                
                // Skip empty lines & comments
                if ((line.length()) && (line.substr(0, 2) != "//"))
                {
                    // Process opening/closing braces
                    if (!ResourceHandler::ProcessBraces(line, brace_level))
                    {
                        // If not a brace and on level 0, it should be a new material; replace name
                        if ((brace_level == 0) && (line.substr(0, 8) == "material"))
                        {
                            if (num_materials == 0)
                            {
                                line = "material " + tempname;
                                ++num_materials;
                            }
                            else
                            {
                                OgreRenderingModule::LogWarning("More than one material defined in material asset " + source->GetId() + " - only first one supported");
                                break;
                            }
                        }
                        else
                        {
                            // Check for textures
                            if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                            {
                                std::string tex_name = line.substr(8);
                                // Note: we assume all texture references are asset based. ResourceHandler checks later whether this is true,
                                // before requesting the reference
                                references_.push_back(Foundation::ResourceReference(tex_name, OgreTextureResource::GetTypeStatic()));
                                original_textures_.push_back(tex_name);
                                // Replace any / with \ in the material, then change the texture names back later, so that Ogre does not go nuts
                                ReplaceCharInplace(line, '/', '\\');
                                ReplaceCharInplace(line, ':', '@');
                            }
                        }

                        // Write line to the modified copy
                        if (!skip_until_next)
                            output << line << std::endl;
                    }
                    else
                    {
                        // Write line to the modified copy
                        if (!skip_until_next)
                            output << line << std::endl;
                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                }
            }

            std::string output_str = output.str();
            Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream((u8 *)(&output_str[0]), output_str.size()));

            matmgr.parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Ogre::MaterialPtr tempmat;
            tempmat = matmgr.getByName(tempname);
            if (tempmat.isNull())
            {
                OgreRenderingModule::LogWarning(std::string("Failed to create an Ogre material from material asset ") +
                    source->GetId());

                return false;
            }
            if(!tempmat->getNumTechniques())
            {
                OgreRenderingModule::LogWarning("Failed to create an Ogre material from material asset "  +
                    source->GetId());
                return false;
            }
            
            ogre_material_ = tempmat->clone(id_);
            tempmat.setNull();
            matmgr.remove(tempname);
            if (ogre_material_.isNull())
            {
                OgreRenderingModule::LogWarning("Failed to create an Ogre material from material asset "  +
                    source->GetId());
                return false;
            }
            
            // Now go through all the texturenames and restore \ back to / and @ to :
            Ogre::Material::TechniqueIterator iter = ogre_material_->getTechniqueIterator();
            while (iter.hasMoreElements())
            {
                Ogre::Technique *tech = iter.getNext();
                Ogre::Technique::PassIterator passIter = tech->getPassIterator();
                while (passIter.hasMoreElements())
                {
                    Ogre::Pass *pass = passIter.getNext();
                    Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                    while (texIter.hasMoreElements())
                    {
                        Ogre::TextureUnitState *texUnit = texIter.getNext();
                        std::string texname = texUnit->getTextureName();
                        if (texname.find('\\') != std::string::npos)
                        {
                            ReplaceCharInplace(texname, '\\', '/');
                            ReplaceCharInplace(texname, '@', ':');
                            texUnit->setTextureName(texname);
                        }
                    }
                }
            }
            
            //workaround: if receives shadows, check the amount of shadowmaps. If only 1 specified, add 2 more to support 3 shadowmaps
            if(ogre_material_->getReceiveShadows() && shadowquality_ == Shadows_High && ogre_material_->getNumTechniques() > 0)
            {
                Ogre::Technique *tech = ogre_material_->getTechnique(0);
                if(tech)
                {
                    Ogre::Technique::PassIterator passiterator = tech->getPassIterator();
                    while(passiterator.hasMoreElements())
                    {
                        Ogre::Pass* pass = passiterator.getNext();
                        Ogre::Pass::TextureUnitStateIterator texiterator = pass->getTextureUnitStateIterator();
                        int shadowmaps = 0;
                        while(texiterator.hasMoreElements())
                        {
                            Ogre::TextureUnitState* state = texiterator.getNext();
                            if(state->getContentType() == Ogre::TextureUnitState::CONTENT_SHADOW)
                            {
                                shadowmaps++;
                            }
                        }
                        if(shadowmaps>0 && shadowmaps<3)
                        {
                            Ogre::TextureUnitState* sm2 = pass->createTextureUnitState();
                            sm2->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);

                            Ogre::TextureUnitState* sm3 = pass->createTextureUnitState();
                            sm3->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);
                        }
                    }
                }

            }

        } catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogWarning(e.what());
            OgreRenderingModule::LogWarning("Failed to parse Ogre material " + source->GetId() + ".");
            try
            {
                if (!matmgr.getByName(tempname).isNull())
                    Ogre::MaterialManager::getSingleton().remove(tempname);
            }
            catch (...) {}
            
            return false;
        }
        return true;
    }

    static const std::string type_name("OgreMaterial");
        
    const std::string& OgreMaterialResource::GetType() const
    {
        return type_name;
    }
    
    const std::string& OgreMaterialResource::GetTypeStatic()
    {
        return type_name;
    }    
    
    void OgreMaterialResource::RemoveMaterial()
    {
        OgreRenderer::RemoveMaterial(ogre_material_);
    }
    
    bool OgreMaterialResource::IsValid() const
    {
        return (!ogre_material_.isNull());
    }
}
