#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreMaterialAsset.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "AssetAPI.h"
#include "TextureAsset.h"

#include "LoggingFunctions.h"

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
    ogreAssetName.clear();
    //original_textures_.clear();

    // Do not go further if in headless mode
    // Ogre will throw exceptions and things go sideways
    if (assetAPI->IsHeadless())
        return false;

    const std::string assetName = this->Name().toStdString();
    Ogre::MaterialManager& matmgr = Ogre::MaterialManager::getSingleton(); 
//    ::LogDebug("Parsing material " + assetName);
    
    if (!data_)
    {
        LogError("DeserializeFromData: Null source asset data pointer");
        return false;
    }
    if (numBytes == 0)
    {
        LogError("DeserializeFromData: Zero sized material asset");
        return false;
    }

    std::string sanitatedname = SanitateAssetIdForOgre(assetName);
    
    std::vector<u8> tempData(data_, data_ + numBytes);
#include "DisableMemoryLeakCheck.h"
    Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&tempData[0], numBytes));
#include "EnableMemoryLeakCheck.h"

    try
    {
        int num_materials = 0;
        int brace_level = 0;
        bool skip_until_next = false;
        int skip_brace_level = 0;
        // Parsed/modified material script
        std::ostringstream output;
        
        while(!data->eof())
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
                            LogWarning("More than one material defined in material asset " + assetName + ". Applying the first found, skipping parsing for others.");
                            break;
                        }
                    }
                    else
                    {
                        // Check for textures
                        if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                        {
                            std::string tex_name = QString(line.substr(8).c_str()).trimmed().toStdString();
                            QString absolute_tex_name = assetAPI->ResolveAssetRef(Name(), tex_name.c_str());
                            references_.push_back(AssetReference(absolute_tex_name));
//                            original_textures_.push_back(tex_name);
                            // Sanitate the asset ID
                            line = "texture " + SanitateAssetIdForOgre(absolute_tex_name.toStdString());
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
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream((u8 *)(&output_str[0]), output_str.size()));
#include "EnableMemoryLeakCheck.h"

        matmgr.parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        ogreMaterial = matmgr.getByName(sanitatedname);
        if (ogreMaterial.isNull())
        {
            LogWarning("DeserializeFromData: Failed to create an Ogre material from material asset: " + assetName);
            return false;
        }        
        if(!ogreMaterial->getNumTechniques())
        {
            LogWarning("DeserializeFromData: Failed to create an Ogre material, no Techniques in material asset: "  + assetName);
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

    } catch(Ogre::Exception &e)
    {
        LogWarning("DeserializeFromData: Failed to parse Ogre material " + assetName + ", reason: " + std::string(e.what()));
        try
        {
            if (!matmgr.getByName(sanitatedname).isNull())
                Ogre::MaterialManager::getSingleton().remove(sanitatedname);
        }
        catch(...) {}
        
        return false;
    }
    
    // Mark the valid ogre resource name
    ogreAssetName = QString::fromStdString(sanitatedname);
    return true;
}

bool OgreMaterialAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    if (ogreMaterial.isNull())
    {
        LogWarning("SerializeTo: Tried to export non-existing Ogre material " + Name().toStdString());
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
        //serializer.exportQueued(filename);
    } 
    catch(std::exception &e)
    {
        ::LogError("SerializeTo: Failed to export Ogre material " + Name().toStdString() + ":");
        if (e.what())
            ::LogError(e.what());
        return false;
    }
    return true;
}

void OgreMaterialAsset::HandleLoadError(const QString &loadError)
{
    // Don't print anything if we are headless, 
    // not loading the material was intentional
    if (!assetAPI->IsHeadless())
        LogError(loadError.toStdString());
}

std::vector<AssetReference> OgreMaterialAsset::FindReferences() const
{
    return references_;
}

bool OgreMaterialAsset::IsLoaded() const
{
    return ogreMaterial.get() != 0;
}

void OgreMaterialAsset::DoUnload()
{
    if (ogreMaterial.isNull())
        return;

    std::string materialName = ogreMaterial->getName();
    ogreMaterial.setNull();

    try
    {
        Ogre::MaterialManager::getSingleton().remove(materialName);
    }
    catch(...) {}
}

Ogre::Technique* OgreMaterialAsset::GetTechnique(int techIndex)
{
    if (ogreMaterial.isNull())
        return 0;
    if (techIndex < 0 || techIndex >= ogreMaterial->getNumTechniques())
        return 0;
    return ogreMaterial->getTechnique(techIndex);
}

Ogre::Pass* OgreMaterialAsset::GetPass(int techIndex, int passIndex)
{
    Ogre::Technique* tech = GetTechnique(techIndex);
    if (!tech)
        return 0;
    if (passIndex < 0 || passIndex >= tech->getNumPasses())
        return 0;
    return tech->getPass(passIndex);
}

Ogre::TextureUnitState* OgreMaterialAsset::GetTextureUnit(int techIndex, int passIndex, int texUnitIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return 0;
    if (texUnitIndex < 0 || texUnitIndex >= pass->getNumTextureUnitStates())
        return 0;
     return pass->getTextureUnitState(texUnitIndex);
}

int OgreMaterialAsset::GetNumTechniques()
{
    if (ogreMaterial.isNull())
        return -1;
    return ogreMaterial->getNumTechniques();
}

int OgreMaterialAsset::GetNumPasses(int techIndex)
{
    Ogre::Technique* tech = GetTechnique(techIndex);
    if (!tech)
        return -1;
    return tech->getNumPasses();
}

int OgreMaterialAsset::GetNumTextureUnits(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return -1;
    return pass->getNumTextureUnitStates();
}

bool OgreMaterialAsset::HasTechnique(int techIndex)
{
    return GetTechnique(techIndex) != 0;
}

bool OgreMaterialAsset::HasPass(int techIndex, int passIndex)
{
    return GetPass(techIndex, passIndex) != 0;
}

int OgreMaterialAsset::CreateTechnique()
{
    if (ogreMaterial.isNull())
        return -1;
    
    try
    {
        Ogre::Technique* newTech = ogreMaterial->createTechnique();
        if (!newTech)
            return -1;
        // Do not assume that the index is the last technique, though that is probably the case. Instead check all
        for (unsigned i = 0; i < ogreMaterial->getNumTechniques(); ++i)
        {
            if (ogreMaterial->getTechnique(i) == newTech)
                return i;
        }
    }
    catch (...)
    {
    }
    
    return -1;
}

int OgreMaterialAsset::CreatePass(int techIndex)
{
    Ogre::Technique* tech = GetTechnique(techIndex);
    if (!tech)
        return -1;
    
    try
    {
        Ogre::Pass* newPass = tech->createPass();
        if (!newPass)
            return -1;
        for (unsigned i = 0; i < tech->getNumPasses(); ++i)
        {
            if (tech->getPass(i) == newPass)
                return i;
        }
    }
    catch (...)
    {
    }
    
    return -1;
}

int OgreMaterialAsset::CreateTextureUnit(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return -1;
    
    try
    {
        Ogre::TextureUnitState* newState = pass->createTextureUnitState();
        if (!newState)
            return -1;
        for (unsigned i = 0; i < pass->getNumTextureUnitStates(); ++i)
        {
            if (pass->getTextureUnitState(i) == newState)
                return i;
        }
    }
    catch (...)
    {
    }
    
    return -1;
}

bool OgreMaterialAsset::RemoveTextureUnit(int techIndex, int passIndex, int texUnitIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    if (texUnitIndex < 0 || texUnitIndex >= pass->getNumTextureUnitStates())
        return false;
    try
    {
        pass->removeTextureUnitState(texUnitIndex);
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}

bool OgreMaterialAsset::RemovePass(int techIndex, int passIndex)
{
    Ogre::Technique* tech = GetTechnique(techIndex);
    if (!tech)
        return false;
    if (passIndex < 0 || passIndex >= tech->getNumPasses())
        return false;
    try
    {
        tech->removePass(passIndex);
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}

bool OgreMaterialAsset::RemoveTechnique(int techIndex)
{
    if (ogreMaterial.isNull())
        return false;
    if (techIndex < 0 || techIndex >= ogreMaterial->getNumTechniques())
        return false;
    try
    {
        ogreMaterial->removeTechnique(techIndex);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool OgreMaterialAsset::SetTexture(int techIndex, int passIndex, int texUnitIndex, AssetPtr texture)
{
    if (!texture)
        return false;
    TextureAsset* texAsset = dynamic_cast<TextureAsset*>(texture.get());
    if (!texAsset || !texAsset->IsLoaded())
        return false;
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    try
    {
        texUnit->setTextureName(texAsset->ogreTexture->getName());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool OgreMaterialAsset::SetShaders(int techIndex, int passIndex, const QString& vertexProgramName, const QString& fragmentProgramName)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    try
    {
        pass->setVertexProgram(vertexProgramName.toStdString());
        pass->setFragmentProgram(fragmentProgramName.toStdString());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool OgreMaterialAsset::SetLighting(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setLightingEnabled(enable);
    return true;
}

bool OgreMaterialAsset::SetDiffuseColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDiffuse(Ogre::ColourValue(color.r, color.g, color.b, color.a));
    return true;
}

bool OgreMaterialAsset::SetAmbientColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setAmbient(Ogre::ColourValue(color.r, color.g, color.b, color.a));
    return true;
}

bool OgreMaterialAsset::SetSpecularColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSpecular(Ogre::ColourValue(color.r, color.g, color.b, color.a));
    return true;
}

bool OgreMaterialAsset::SetEmissiveColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSelfIllumination(Ogre::ColourValue(color.r, color.g, color.b, color.a));
    return true;
}

bool OgreMaterialAsset::SetAlphaBlend(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    return true;
}

bool OgreMaterialAsset::SetAdditive(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending(Ogre::SBT_ADD);
    return true;
}

bool OgreMaterialAsset::SetModulate(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending(Ogre::SBT_MODULATE);
    return true;
}

bool OgreMaterialAsset::SetReplace(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending(Ogre::SBT_REPLACE);
    return true;
}

bool OgreMaterialAsset::SetSolid(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setPolygonMode(Ogre::PM_SOLID);
    return true;
}

bool OgreMaterialAsset::SetWireframe(int techIndex, int passIndex)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setPolygonMode(Ogre::PM_WIREFRAME);
    return true;
}

bool OgreMaterialAsset::SetDepthWrite(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthWriteEnabled(enable);
    return true;
}

bool OgreMaterialAsset::SetDepthBias(int techIndex, int passIndex, float bias)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthBias(bias);
    return true;
}
