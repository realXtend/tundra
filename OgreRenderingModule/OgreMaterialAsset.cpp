#include "StableHeaders.h"
#include "OgreMaterialAsset.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "Renderer.h"

using namespace OgreRenderer;

OgreMaterialAsset::~OgreMaterialAsset()
{
    Unload();
}

bool OgreMaterialAsset::DeserializeFromData(const u8 *data_, size_t numBytes)
{
    // Remove old material if any
    Unload();
    references_.clear();
//    original_textures_.clear();

    const std::string assetName = this->Name().toStdString();

    Ogre::MaterialManager& matmgr = Ogre::MaterialManager::getSingleton(); 

    OgreRenderingModule::LogDebug("Parsing material " + assetName);
    
    if (!data_)
    {
        OgreRenderingModule::LogError("Null source asset data pointer");
        return false;
    }
    if (numBytes == 0)
    {
        OgreRenderingModule::LogError("Zero sized material asset");
        return false;
    }

    std::string sanitatedname = SanitateAssetIdForOgre(assetName);
    
    std::vector<u8> tempData(data_, data_ + numBytes);
    Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&tempData[0], numBytes));

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
                if (!ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new material; replace name
                    if ((brace_level == 0) && (line.substr(0, 8) == "material"))
                    {
                        if (num_materials == 0)
                        {
                            line = "material " + sanitatedname;
                            ++num_materials;
                        }
                        else
                        {
                            OgreRenderingModule::LogWarning("More than one material defined in material asset " + assetName + " - only first one supported");
                            break;
                        }
                    }
                    else
                    {
                        // Check for textures
                        if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                        {
                            std::string tex_name = QString(line.substr(8).c_str()).trimmed().toStdString();
                            // Note: we assume all texture references are asset based. ResourceHandler checks later whether this is true,
                            // before requesting the reference
                            references_.push_back(AssetReference(tex_name.c_str()));
//                            original_textures_.push_back(tex_name);
                            // Sanitate the asset ID
                            line = SanitateAssetIdForOgre(line);
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
        ogreMaterial = matmgr.getByName(sanitatedname);
        if (ogreMaterial.isNull())
        {
            OgreRenderingModule::LogWarning(std::string("Failed to create an Ogre material from material asset ") +
                assetName);

            return false;
        }
        if(!ogreMaterial->getNumTechniques())
        {
            OgreRenderingModule::LogWarning("Failed to create an Ogre material from material asset "  +
                assetName);
            ogreMaterial.setNull();
            return false;
        }
        
        ShadowQuality shadowquality_ = Shadows_High; ///\todo Regression. Read this ahead of time.

        //workaround: if receives shadows, check the amount of shadowmaps. If only 1 specified, add 2 more to support 3 shadowmaps
        if(ogreMaterial->getReceiveShadows() && shadowquality_ == Shadows_High && ogreMaterial->getNumTechniques() > 0)
        {
            Ogre::Technique *tech = ogreMaterial->getTechnique(0);
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
        OgreRenderingModule::LogWarning("Failed to parse Ogre material " + assetName + ".");
        try
        {
            if (!matmgr.getByName(sanitatedname).isNull())
                Ogre::MaterialManager::getSingleton().remove(sanitatedname);
        }
        catch (...) {}
        
        return false;
    }
    
//    internal_name_ = sanitatedname;
    return true;
}

bool OgreMaterialAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters)
{
    if (ogreMaterial.isNull())
    {
        OgreRenderingModule::LogWarning("Tried to export non-existing Ogre material " + Name().toStdString() + ".");
        return false;
    }
    try
    {
        Ogre::MaterialSerializer serializer;
        serializer.queueForExport(ogreMaterial);
        std::string materialData = serializer.getQueuedAsString();
        if (materialData.empty())
            return false;

        data.clear();
        data.insert(data.end(), &materialData[0], &materialData[0] + materialData.length());

//        serializer.exportQueued(filename);
    } catch (std::exception &e)
    {
        OgreRenderingModule::LogError("Failed to export Ogre material " + Name().toStdString() + ":");
        if (e.what())
            OgreRenderingModule::LogError(e.what());
        return false;
    }
    return true;
}

std::vector<AssetReference> OgreMaterialAsset::FindReferences() const
{
    return references_;
}

void OgreMaterialAsset::Unload()
{
    if (ogreMaterial.isNull())
        return;

    std::string materialName = ogreMaterial->getName();
    ogreMaterial.setNull();

    try
    {
        Ogre::MaterialManager::getSingleton().remove(materialName);
    }
    catch (...) {}
}
