// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreConversionUtils.h"
#include "OgreParticleAsset.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "AssetAPI.h"

#include <Ogre.h>

using namespace OgreRenderer;

void ModifyVectorParameter(Ogre::String& line, std::vector<Ogre::String>& line_vec);

OgreParticleAsset::~OgreParticleAsset()
{
    Unload();
}

std::vector<AssetReference> OgreParticleAsset::FindReferences() const
{
    return references_;
}

void OgreParticleAsset::DoUnload()
{
    RemoveTemplates();
}

bool OgreParticleAsset::DeserializeFromData(const u8 *data_, size_t numBytes)
{
    RemoveTemplates();
    references_.clear();

    if (!data_)
    {
        OgreRenderer::OgreRenderingModule::LogError("Null source asset data pointer");     
        return false;
    }
    if (numBytes == 0)
    {
        OgreRenderer::OgreRenderingModule::LogError("Zero sized particle system asset");     
        return false;
    }

    // Detected template names
    StringVector new_templates;

    std::vector<u8> tempData(data_, data_ + numBytes);
    Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&tempData[0], numBytes));
    try
    {
        int brace_level = 0;
        bool skip_until_next = false;
        int skip_brace_level = 0;
        // Parsed/modified script
        std::ostringstream output;

        while (!data->eof())
        {
            Ogre::String line = data->getLine();
            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Split line to components
                std::vector<Ogre::String> line_vec;

#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR == 6 
		      line_vec = Ogre::StringUtil::split(line, "\t ");
#else 
		      Ogre::vector<Ogre::String>::type vec = Ogre::StringUtil::split(line,"\t ");
		      int size = vec.size();
		      line_vec.resize(size);
		      
		      for (int i = 0; i < size; ++i)
			line_vec[i] = vec[i];
#endif               

                // Check for vector parameters to be modified, so that particle scripts can be authored in typical Ogre coord system
                ModifyVectorParameter(line, line_vec);

                // Process opening/closing braces
                if (!ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new particlesystem; replace name with resource ID + ordinal
                    if (brace_level == 0)
                    {
                        line = SanitateAssetIdForOgre(this->Name().toStdString()) + "_" + boost::lexical_cast<std::string>(new_templates.size());
                        new_templates.push_back(line);
                        // New script compilers need this
                        line = "particle_system " + line;
                    }
                    else
                    {
                        // Check for ColourImage, which is a risky affector and may easily crash if image can't be loaded
                        if (line_vec[0] == "affector")
                        {   
                           if (line_vec.size() >= 2)
                            {
                                if (line_vec[1] == "ColourImage")
                                {
                                    skip_until_next = true;
                                    skip_brace_level = brace_level;
                                }
                            }
                        }
                        // Check for material definition
                        else if (line_vec[0] == "material")
                        {             
                            if (line_vec.size() >= 2)
                            {
                                // Tundra: we only support material refs in particle scripts
                                std::string mat_name = line_vec[1];
                                ///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
                                references_.push_back(AssetReference(assetAPI->LookupAssetRefToStorage(mat_name.c_str())));
                                line = "material " + SanitateAssetIdForOgre(mat_name);
                            }
                        }
                    }
                    // Write line to the copy
                    if (!skip_until_next)
                    {
                        output << line << std::endl;
                    }
                    else
                        OgreRenderer::OgreRenderingModule::LogDebug("Skipping risky particle effect line: " + line);
                }
                else
                {
                    // Write line to the copy
                    if (!skip_until_next)
                    {
                        output << line << std::endl;
                    }
                    else
                        OgreRenderer::OgreRenderingModule::LogDebug("Skipping risky particle effect line: " + line);

                    if (brace_level <= skip_brace_level)
                        skip_until_next = false;
                }
            } 
        }

        std::string output_str = output.str();
        Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&output_str[0], output_str.size()));
        Ogre::ParticleSystemManager::getSingleton().parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    catch (Ogre::Exception& e)
    {
        OgreRenderer::OgreRenderingModule::LogWarning(e.what());
        OgreRenderer::OgreRenderingModule::LogWarning("Failed to parse Ogre particle script " + Name().toStdString() + ".");
    }
    
    // Check which templates actually succeeded
    for (uint i = 0; i < new_templates.size(); ++i)
    {
        if (Ogre::ParticleSystemManager::getSingleton().getTemplate(new_templates[i]))
        {
            templates_.push_back(new_templates[i]);
            OgreRenderer::OgreRenderingModule::LogDebug("Ogre particle system template " + new_templates[i] + " created");
        }
    }
    
    // Give only the name of the first template
    internal_name_ = SanitateAssetIdForOgre(Name().toStdString()) + "_0";
    
    // Theoretical success if at least one template was created
    return GetNumTemplates() > 0;
}

int OgreParticleAsset::GetNumTemplates() const
{
    return templates_.size();
}

QString OgreParticleAsset::GetTemplateName(int index) const
{
    if (index >= templates_.size())
        return "";
        
    return templates_[index].c_str();
}

void OgreParticleAsset::RemoveTemplates()
{
    for (unsigned i = 0; i < templates_.size(); ++i)
    {
        try
        {
            Ogre::ParticleSystemManager::getSingleton().removeTemplate(templates_[i]);
        } catch (...) {}
    }
    templates_.clear();
}

void ModifyVectorParameter(Ogre::String& line, std::vector<Ogre::String>& line_vec)
{
    static const std::string modify_these[] = {"position", "direction", "force_vector", "common_direction", "common_up_vector", "plane_point", "plane_normal", ""};
    
    // Line should consist of the command & 3 values
    if (line_vec.size() != 4)
        return;
    
    for (uint i = 0; modify_these[i].length(); ++i)
    {
        if (line_vec[0] == modify_these[i])
        {   
            try
            {
                float x = ParseString<float>(line_vec[1]);
                float y = ParseString<float>(line_vec[2]);
                float z = ParseString<float>(line_vec[3]);

                // For compatibility with old rex assets, the Z coord has to be reversed
                std::stringstream s;
                s << line_vec[0] << " " << x << " " << y << " " << -z;
                // Write back the modified string
                line = s.str();
            }
            catch (...)
            {
            }
            return;
        }        
    }
}
