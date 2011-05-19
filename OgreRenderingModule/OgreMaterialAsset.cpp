#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreMaterialAsset.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "AssetAPI.h"
#include "TextureAsset.h"
#include "IAssetTransfer.h"
#include "XMLUtilities.h"

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

struct EnumStr
{
    EnumStr(QString name, unsigned value) :
        name_(name),
        value_(value)
    {
    }
    
    EnumStr() :
        value_(0xffffffff)
    {
    }
    
    QString name_;
    unsigned value_;
};

EnumStr sceneBlendTypes[] =
{
    EnumStr("replace", Ogre::SBT_REPLACE),
    EnumStr("add", Ogre::SBT_ADD),
    EnumStr("modulate", Ogre::SBT_MODULATE),
    EnumStr("alpha_blend", Ogre::SBT_TRANSPARENT_ALPHA),
    EnumStr("colour_blend", Ogre::SBT_TRANSPARENT_COLOUR),
    EnumStr()
};

EnumStr sceneBlendFactors[] = 
{
    EnumStr("one", Ogre::SBF_ONE),
    EnumStr("zero", Ogre::SBF_ZERO),
    EnumStr("dest_colour", Ogre::SBF_DEST_COLOUR),
    EnumStr("src_colour", Ogre::SBF_SOURCE_COLOUR),
    EnumStr("one_minus_dest_colour", Ogre::SBF_ONE_MINUS_DEST_COLOUR),
    EnumStr("one_minus_src_colour", Ogre::SBF_ONE_MINUS_SOURCE_COLOUR),
    EnumStr("dest_alpha", Ogre::SBF_DEST_ALPHA),
    EnumStr("src_alpha", Ogre::SBF_SOURCE_ALPHA),
    EnumStr("one_minus_dest_alpha", Ogre::SBF_ONE_MINUS_DEST_ALPHA),
    EnumStr("one_minus_src_alpha", Ogre::SBF_ONE_MINUS_SOURCE_ALPHA),
    EnumStr()
};

EnumStr compareFunctions[] =
{
    EnumStr("always_fail", Ogre::CMPF_ALWAYS_FAIL),
    EnumStr("always_pass", Ogre::CMPF_ALWAYS_PASS),
    EnumStr("less", Ogre::CMPF_LESS),
    EnumStr("less_equal", Ogre::CMPF_LESS_EQUAL),
    EnumStr("equal", Ogre::CMPF_EQUAL),
    EnumStr("not_equal", Ogre::CMPF_NOT_EQUAL),
    EnumStr("greater_equal", Ogre::CMPF_GREATER_EQUAL),
    EnumStr("greater", Ogre::CMPF_GREATER),
    EnumStr()
};

EnumStr sceneBlendOps[] =
{
    EnumStr("add", Ogre::SBO_ADD),
    EnumStr("subtract", Ogre::SBO_SUBTRACT),
    EnumStr("reverse_subtract", Ogre::SBO_REVERSE_SUBTRACT),
    EnumStr("min", Ogre::SBO_MIN),
    EnumStr("max", Ogre::SBO_MAX),
    EnumStr()
};

EnumStr cullingModes[] =
{
    EnumStr("clockwise", Ogre::CULL_CLOCKWISE),
    EnumStr("anticlockwise", Ogre::CULL_ANTICLOCKWISE),
    EnumStr("none", Ogre::CULL_NONE),
    EnumStr()
};

EnumStr shadingModes[] =
{
    EnumStr("flat", Ogre::SO_FLAT),
    EnumStr("gouraud", Ogre::SO_GOURAUD),
    EnumStr("phong", Ogre::SO_PHONG),
    EnumStr()
};

EnumStr polygonModes[] =
{
    EnumStr("solid", Ogre::PM_SOLID),
    EnumStr("wireframe", Ogre::PM_WIREFRAME),
    EnumStr("points", Ogre::PM_POINTS),
    EnumStr()
};

EnumStr texAddressModes[] =
{
    EnumStr("wrap", Ogre::TextureUnitState::TAM_WRAP),
    EnumStr("clamp", Ogre::TextureUnitState::TAM_CLAMP),
    EnumStr("mirror", Ogre::TextureUnitState::TAM_MIRROR),
    EnumStr("border", Ogre::TextureUnitState::TAM_BORDER),
    EnumStr()
};

EnumStr texFilterOptions[] =
{
    EnumStr("none", Ogre::TFO_NONE),
    EnumStr("bilinear", Ogre::TFO_BILINEAR),
    EnumStr("trilinear", Ogre::TFO_TRILINEAR),
    EnumStr("anisotropic", Ogre::TFO_ANISOTROPIC),
    EnumStr()
};

EnumStr envMapTypes[] =
{
    EnumStr("spherical", Ogre::TextureUnitState::ENV_CURVED),
    EnumStr("planar", Ogre::TextureUnitState::ENV_PLANAR),
    EnumStr("cubic_reflection", Ogre::TextureUnitState::ENV_REFLECTION),
    EnumStr("cubic_normal", Ogre::TextureUnitState::ENV_NORMAL),
    EnumStr()
};

EnumStr textureTransformTypes[] =
{
    EnumStr("scroll_x", Ogre::TextureUnitState::TT_TRANSLATE_U),
    EnumStr("scroll_y", Ogre::TextureUnitState::TT_TRANSLATE_V),
    EnumStr("rotate", Ogre::TextureUnitState::TT_ROTATE),
    EnumStr("scale_x", Ogre::TextureUnitState::TT_SCALE_U),
    EnumStr("scale_y", Ogre::TextureUnitState::TT_SCALE_V),
    EnumStr()
};

EnumStr waveformTypes[] =
{
    EnumStr("sine", Ogre::WFT_SINE),
    EnumStr("triangle", Ogre::WFT_TRIANGLE),
    EnumStr("square", Ogre::WFT_SQUARE),
    EnumStr("sawtooth", Ogre::WFT_SAWTOOTH),
    EnumStr("inverse_sawtooth", Ogre::WFT_INVERSE_SAWTOOTH),
    EnumStr()
};

// Return value from an enum table. First (default) value will be returned if no match found
unsigned GetEnumValue(const QString& name, EnumStr* enums)
{
    EnumStr* ptr = enums;
    while (ptr && ptr->name_.length() > 0)
    {
        if (ptr->name_ == name)
            return ptr->value_;
        ++ptr;
    }
    
    return enums->value_;
}

// Convert lowercase text to bool. Accepted variations are on/off, true/false & 0/1
bool GetBoolValue(const QString& value)
{
    if (value.isEmpty())
        return false;
    if (value == "1")
        return true;
    if (value == "on")
        return true;
    if (value == "true")
        return true;
    return false;
}

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

void OgreMaterialAsset::CopyContent(AssetPtr source)
{
    // Not supported in headless mode
    if (assetAPI->IsHeadless())
        return;
        
    if (!source)
    {
        LogError("CopyContent: Null source");
        return;
    }
    OgreMaterialAsset* sourceMat = dynamic_cast<OgreMaterialAsset*>(source.get());
    if (!sourceMat)
    {
        LogError("CopyContent: Source is not an OgreMaterialAsset");
        return;
    }
    if (sourceMat == this)
    {
        LogWarning("CopyContent: Source is same as destination, cannot copy");
        return;
    }
    if (sourceMat->ogreMaterial.isNull())
    {
        LogError("CopyContent: source " + source->Name() + " is unloaded");
        return;
    }
    
    // If we are unloaded, create an empty material first
    if (ogreMaterial.isNull())
    {
        try
        {
            std::string sanitatedName = SanitateAssetIdForOgre(Name());
            ogreMaterial = Ogre::MaterialManager::getSingleton().create(sanitatedName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        }
        catch (Ogre::Exception& e)
        {
            LogError("CopyContent: Failed to create empty material " + Name().toStdString() + ", reason: " + e.what());
            return;
        }
    }
    
    Ogre::Material* sourceOgreMat = sourceMat->ogreMaterial.get();
    
    // Then copy parameters & techniques
    ogreMaterial->setReceiveShadows(sourceOgreMat->getReceiveShadows());
    ogreMaterial->setTransparencyCastsShadows(sourceOgreMat->getTransparencyCastsShadows());
    ogreMaterial->removeAllTechniques();
    for (unsigned short i = 0; i < sourceOgreMat->getNumTechniques(); ++i)
    {
        Ogre::Technique* sourceTech = sourceOgreMat->getTechnique(i);
        if (sourceTech)
        {
            Ogre::Technique* destTech = ogreMaterial->createTechnique();
            *destTech = *sourceTech;
        }
    }
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

void OgreMaterialAsset::SetAttribute(const QString& key, const QString& value)
{
    // Material must exist
    if (!IsLoaded())
        return;
    
    QStringList keyParts = key.toLower().split(' ');
    if (!keyParts.size())
        return;
    
    QString attr = keyParts.back();
    QString trimmedValue = value.trimmed().toLower(); // trimmed and converted to lowercase. However also retain original value for asset refs etc.
    
    // If we are setting a material-level attribute, no need to scan for technique/pass/textureunit
    if (SetMaterialAttribute(attr, trimmedValue, value))
        return;
    
    // Identify technique, pass & texture unit. The default is 'any'.
    int techNum = -1; // -1 = any
    int passNum = -1; 
    int tuNum = -1;
    
    for (int i = 0; i < (int)keyParts.size() - 1; ++i)
    {
        if (keyParts[i].size() > 1)
        {
            // Technique or texture unit setting
            if (keyParts[i][0] == 't')
            {
                if (keyParts[i].size() > 2)
                {
                    if (keyParts[i][1] == 'u')
                        tuNum = keyParts[i].mid(2).toInt();
                    else
                        techNum = keyParts[i].mid(1).toInt();
                }
                else
                    techNum = keyParts[i].mid(1).toInt();
            }
            // Pass setting
            else if (keyParts[i][0] == 'p')
                passNum = keyParts[i].mid(1).toInt();
        }
    }
    
    int firstTech = 0;
    int lastTech = GetNumTechniques() - 1;
    if (techNum != -1)
    {
        firstTech = techNum;
        lastTech = techNum;
    }
    
    for (int t = firstTech; t <= lastTech; ++t)
    {
        // Handle technique-level attribute
        Ogre::Technique* tech = GetTechnique(t);
        if (!tech)
            continue;
        
        if (SetTechniqueAttribute(tech, t, attr, trimmedValue, value))
            continue;
        
        int firstPass = 0;
        int lastPass = GetNumPasses(t) - 1;
        if (passNum != -1)
        {
            firstPass = passNum;
            lastPass = passNum;
        }
        
        for (int p = firstPass; p <= lastPass; ++p)
        {
            Ogre::Pass* pass = GetPass(t, p);
            if (!pass)
                continue;
            
            // Handle pass-level attribute
            if (SetPassAttribute(pass, t, p, attr, trimmedValue, value))
                continue;
            
            int firstTu = 0;
            int lastTu = GetNumTextureUnits(t, p);
            if (tuNum != -1)
            {
                firstTu = tuNum;
                lastTu = tuNum;
            }
            
            for (int tu = firstTu; tu <= lastTu; ++tu)
            {
                Ogre::TextureUnitState* texUnit = GetTextureUnit(t, p, tu);
                if (!texUnit)
                    continue;
                // Handle textureunit-level attribute
                SetTextureUnitAttribute(texUnit, t, p, tu, attr, trimmedValue, value);
            }
        }
    }
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

bool OgreMaterialAsset::SetTexture(int techIndex, int passIndex, int texUnitIndex, const QString& assetRef)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    
    // Calculate the absolute reference index so that we know which reference to change. This requires going through all the texture units up to this.
    int refIndex = 0;
    bool refIndexFound = false;
    for (int t = 0; (!refIndexFound) && t <= techIndex; ++t)
    {
        for (int p = 0; (!refIndexFound) && p < GetNumPasses(t); ++p)
        {
            for (int tu = 0; (!refIndexFound) && tu < GetNumTextureUnits(t, p); ++tu)
            {
                Ogre::TextureUnitState* texUnit2 = GetTextureUnit(t, p, tu);
                if (texUnit2 == texUnit)
                {
                    refIndexFound = true;
                    break;
                }
                // Note: we may also have no-texture textureunits. Don't count them
                else if ((texUnit2) && (!texUnit2->getTextureName().empty()))
                    ++refIndex;
            }
        }
    }
    
    // If the reference vector does not have enough space (new texture assignment to a previously no-texture unit, add new slot
    if (refIndex >= (int)references_.size())
    {
        references_.resize(references_.size() + 1);
        refIndex = references_.size() - 1;
    }
    
    QString resolvedRef = assetAPI->ResolveAssetRef(Name(), assetRef);
    references_[refIndex] = AssetReference(resolvedRef);
    
    // Easy case: try to get the asset directly
    AssetPtr asset = assetAPI->GetAsset(resolvedRef);
    if (asset)
    {
        TextureAsset* texAsset = dynamic_cast<TextureAsset*>(asset.get());
        if (texAsset && texAsset->IsLoaded())
        {
            try
            {
                texUnit->setTextureName(texAsset->ogreTexture->getName());
            }
            catch (Ogre::Exception& e)
            {
                LogError("SetTexture exception for " + Name().toStdString() + ", reason: " + std::string(e.what()));
                return false;
            }
            return true;
        }
    }
    
    // Asset was not found. Request it and fill a pending refs entry so that we can apply it once it gets loaded
    AssetTransferPtr transfer = assetAPI->RequestAsset(resolvedRef);
    if (transfer)
    {
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnTransferSucceeded(AssetPtr)), Qt::UniqueConnection);
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), this, SLOT(OnTransferFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
        
        PendingTextureApply newApply;
        newApply.transfer = transfer.get(); // To store a raw pointer is intentional. We never dereference the stored transfer, only compare to it in slot functions
        newApply.techIndex = techIndex;
        newApply.passIndex = passIndex;
        newApply.tuIndex = texUnitIndex;
        pendingApplies.push_back(newApply);
    }
    
    return true;
}

bool OgreMaterialAsset::SetVertexShader(int techIndex, int passIndex, const QString& vertexShaderName)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    try
    {
        pass->setVertexProgram(vertexShaderName.toStdString());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool OgreMaterialAsset::SetPixelShader(int techIndex, int passIndex, const QString& pixelShaderName)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    try
    {
        pass->setFragmentProgram(pixelShaderName.toStdString());
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
    pass->setDiffuse(ToOgreColor(color));
    return true;
}

bool OgreMaterialAsset::SetAmbientColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setAmbient(ToOgreColor(color));
    return true;
}

bool OgreMaterialAsset::SetSpecularColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSpecular(ToOgreColor(color));
    return true;
}

bool OgreMaterialAsset::SetEmissiveColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSelfIllumination(ToOgreColor(color));
    return true;
}

bool OgreMaterialAsset::SetSceneBlend(int techIndex, int passIndex, unsigned blendMode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending((Ogre::SceneBlendType)blendMode);
    return true;
}

bool OgreMaterialAsset::SetPolygonMode(int techIndex, int passIndex, unsigned polygonMode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setPolygonMode((Ogre::PolygonMode)polygonMode);
    return true;
}

bool OgreMaterialAsset::SetDepthCheck(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthCheckEnabled(enable);
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


bool OgreMaterialAsset::SetMaterialAttribute(const QString& attr, const QString& val, const QString& origVal)
{
    if (attr == "receive_shadows")
    {
        ogreMaterial->setReceiveShadows(GetBoolValue(val));
        return true;
    }
    if (attr == "transparency_casts_shadows")
    {
        ogreMaterial->setTransparencyCastsShadows(GetBoolValue(val));
        return true;
    }
    return false;
}

bool OgreMaterialAsset::SetTechniqueAttribute(Ogre::Technique* tech, int techIndex, const QString& attr, const QString& val, const QString& origVal)
{
    /// \todo Material asset requests
    if (attr == "shadow_caster_material")
    {
        tech->setShadowCasterMaterial(origVal.toStdString());
        return true;
    }
    if (attr == "shadow_receiver_material")
    {
        tech->setShadowReceiverMaterial(origVal.toStdString());
        return true;
    }
    
    return false;
}

bool OgreMaterialAsset::SetPassAttribute(Ogre::Pass* pass, int techIndex, int passIndex, const QString& attr, const QString& val, const QString& origVal)
{
    if (attr == "ambient")
    {
        SetAmbientColor(techIndex, passIndex, ParseColor(val.toStdString()));
        return true;
    }
    if (attr == "diffuse")
    {
        SetDiffuseColor(techIndex, passIndex, ParseColor(val.toStdString()));
        return true;
    }
    if (attr == "specular")
    {
        SetSpecularColor(techIndex, passIndex, ParseColor(val.toStdString()));
        return true;
    }
    if (attr == "emissive")
    {
        SetEmissiveColor(techIndex, passIndex, ParseColor(val.toStdString()));
        return true;
    }
    if (attr == "scene_blend")
    {
        // Check whether value is simple or complex
        QStringList values = val.split(' ');
        if (values.size() < 2)
            SetSceneBlend(techIndex, passIndex, GetEnumValue(val, sceneBlendTypes));
        else
            pass->setSceneBlending((Ogre::SceneBlendFactor)GetEnumValue(values[0], sceneBlendFactors), (Ogre::SceneBlendFactor)GetEnumValue(values[1], sceneBlendFactors));
        return true;
    }
    if (attr == "separate_scene_blend")
    {
        // Check whether value is simple or complex
        QStringList values = val.split(' ');
        if (values.size() == 2)
            pass->setSeparateSceneBlending((Ogre::SceneBlendType)GetEnumValue(values[0], sceneBlendTypes), (Ogre::SceneBlendType)GetEnumValue(values[1], sceneBlendTypes));
        else if (values.size() == 4)
            pass->setSeparateSceneBlending((Ogre::SceneBlendFactor)GetEnumValue(values[0], sceneBlendFactors), (Ogre::SceneBlendFactor)GetEnumValue(values[1], sceneBlendFactors),
                (Ogre::SceneBlendFactor)GetEnumValue(values[2], sceneBlendFactors), (Ogre::SceneBlendFactor)GetEnumValue(values[3], sceneBlendFactors));
        return true;
    }
    if (attr == "scene_blend_op")
    {
        pass->setSceneBlendingOperation((Ogre::SceneBlendOperation)GetEnumValue(val, sceneBlendOps));
        return true;
    }
    if (attr == "separate_scene_blend_op")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 2)
            pass->setSeparateSceneBlendingOperation((Ogre::SceneBlendOperation)GetEnumValue(values[0], sceneBlendOps), (Ogre::SceneBlendOperation)GetEnumValue(values[1], sceneBlendOps));
        return true;
    }
    if (attr == "depth_check")
    {
        SetDepthCheck(techIndex, passIndex, GetBoolValue(val));
        return true;
    }
    if (attr == "depth_write")
    {
        SetDepthWrite(techIndex, passIndex, GetBoolValue(val));
        return true;
    }
    if (attr == "depth_func")
    {
        pass->setDepthFunction((Ogre::CompareFunction)GetEnumValue(val, compareFunctions));
        return true;
    }
    if (attr == "depth_bias")
    {
        QStringList values = val.split(' ');
        if (values.size() < 2)
            pass->setDepthBias(val.toFloat());
        else
            pass->setDepthBias(values[0].toFloat(), values[1].toFloat());
        return true;
    }
    if (attr == "alpha_rejection")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 2)
        {
            pass->setAlphaRejectFunction((Ogre::CompareFunction)GetEnumValue(values[0], compareFunctions));
            pass->setAlphaRejectValue(values[1].toUInt());
        }
        return true;
    }
    if (attr == "normalise_normals")
    {
        pass->setNormaliseNormals(GetBoolValue(val));
        return true;
    }
    if (attr == "transparent_sorting")
    {
        pass->setTransparentSortingEnabled(GetBoolValue(val));
        return true;
    }
    if (attr == "cull_hardware")
    {
        pass->setCullingMode((Ogre::CullingMode)GetEnumValue(val, cullingModes));
        return true;
    }
    if (attr == "lighting")
    {
        SetLighting(techIndex, passIndex, GetBoolValue(val));
        return true;
    }
    if (attr == "shading")
    {
        pass->setShadingMode((Ogre::ShadeOptions)GetEnumValue(val, shadingModes));
        return true;
    }
    if (attr == "polygon_mode")
    {
        SetPolygonMode(techIndex, passIndex, (Ogre::PolygonMode)GetEnumValue(val, polygonModes));
        return true;
    }
    if (attr == "colour_write")
    {
        pass->setColourWriteEnabled(GetBoolValue(val));
        return true;
    }
    if (attr == "vertex_program_ref")
    {
        SetVertexShader(techIndex, passIndex, origVal);
        return true;
    }
    if (attr == "fragment_program_ref")
    {
        SetPixelShader(techIndex, passIndex, origVal);
        return true;
    }
    return false;
}

bool OgreMaterialAsset::SetTextureUnitAttribute(Ogre::TextureUnitState* texUnit, int techIndex, int passIndex, int tuIndex, const QString& attr, const QString& val, const QString& origVal)
{
    if (attr == "texture")
    {
        SetTexture(techIndex, passIndex, tuIndex, origVal);
        return true;
    }
    if (attr == "tex_coord_set")
    {
        texUnit->setTextureCoordSet(val.toInt());
        return true;
    }
    if (attr == "tex_address_mode")
    {
        QStringList values = val.split(' ');
        if (values.size() < 2)
            texUnit->setTextureAddressingMode((Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(val, texAddressModes));
        else
        {
            if (values.size() >= 3)
                texUnit->setTextureAddressingMode(
                    (Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(values[0], texAddressModes),
                    (Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(values[1], texAddressModes),
                    (Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(values[2], texAddressModes));
            else
                texUnit->setTextureAddressingMode(
                    (Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(values[0], texAddressModes),
                    (Ogre::TextureUnitState::TextureAddressingMode)GetEnumValue(values[1], texAddressModes),
                    Ogre::TextureUnitState::TAM_WRAP);
        }
        return true;
    }
    if (attr == "tex_border_colour")
    {
        texUnit->setTextureBorderColour(ToOgreColor(ParseColor(val.toStdString())));
        return true;
    }
    
    if (attr == "filtering")
    {
        texUnit->setTextureFiltering((Ogre::TextureFilterOptions)GetEnumValue(val, texFilterOptions));
        return true;
    }
    if (attr == "max_anisotropy")
    {
        texUnit->setTextureAnisotropy(val.toUInt());
        return true;
    }
    if (attr == "mipmap_bias")
    {
        texUnit->setTextureMipmapBias(val.toFloat());
        return true;
    }
    if (attr == "env_map")
    {
        if (val != "off")
            texUnit->setEnvironmentMap(true, (Ogre::TextureUnitState::EnvMapType)GetEnumValue(val, envMapTypes));
        else
            texUnit->setEnvironmentMap(false);
        return true;
    }
    if (attr == "scroll")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 2)
            texUnit->setTextureScroll(values[0].toFloat(), values[1].toFloat());
        return true;
    }
    if (attr == "scroll_anim")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 2)
            texUnit->setScrollAnimation(values[0].toFloat(), values[1].toFloat());
        return true;
    }
    if (attr == "rotate")
    {
        texUnit->setTextureRotate(Ogre::Degree(val.toFloat()));
        return true;
    }
    if (attr == "rotate_anim")
    {
        texUnit->setRotateAnimation(val.toFloat());
        return true;
    }
    if (attr == "scale")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 2)
            texUnit->setTextureScale(values[0].toFloat(), values[1].toFloat());
        return true;
    }
    if (attr == "wave_xform")
    {
        QStringList values = val.split(' ');
        if (values.size() >= 6)
            texUnit->setTransformAnimation((Ogre::TextureUnitState::TextureTransformType)GetEnumValue(values[0], textureTransformTypes), 
            (Ogre::WaveformType)GetEnumValue(values[1], waveformTypes), values[2].toFloat(), values[3].toFloat(), values[4].toFloat(), values[5].toFloat());
        return true;
    }
    
    return false;
}

void OgreMaterialAsset::OnTransferSucceeded(AssetPtr asset)
{
    IAssetTransfer* transfer = static_cast<IAssetTransfer*>(sender());
    for (unsigned i = pendingApplies.size() - 1; i < pendingApplies.size(); --i)
    {
        PendingTextureApply& entry = pendingApplies[i];
        if (entry.transfer == transfer)
        {
            TextureAsset* texAsset = dynamic_cast<TextureAsset*>(asset.get());
            if (texAsset && texAsset->IsLoaded())
            {
                Ogre::TextureUnitState* texUnit = GetTextureUnit(entry.techIndex, entry.passIndex, entry.tuIndex);
                if (texUnit)
                {
                    try
                    {
                        texUnit->setTextureName(texAsset->ogreTexture->getName());
                    }
                    catch (Ogre::Exception& e)
                    {
                        LogError("SetTexture exception for " + Name().toStdString() + ", reason: " + std::string(e.what()));
                    }
                }
            }
            pendingApplies.erase(pendingApplies.begin() + i);
        }
    }
}

void OgreMaterialAsset::OnTransferFailed(IAssetTransfer* transfer, QString reason)
{
    for (unsigned i = pendingApplies.size() - 1; i < pendingApplies.size(); --i)
    {
        PendingTextureApply& entry = pendingApplies[i];
        if (entry.transfer == transfer)
        {
            Ogre::TextureUnitState* texUnit = GetTextureUnit(entry.techIndex, entry.passIndex, entry.tuIndex);
            if (texUnit)
            {
                try
                {
                    texUnit->setTextureName("AssetLoadError.png");
                }
                catch (Ogre::Exception& e)
                {
                    LogError("SetTexture exception for " + Name().toStdString() + ", reason: " + std::string(e.what()));
                }
            }
            pendingApplies.erase(pendingApplies.begin() + i);
        }
    }
}
