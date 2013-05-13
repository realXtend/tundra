// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"
#include "OgreMaterialAsset.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialUtils.h"
#include "Renderer.h"
#include "AssetAPI.h"
#include "TextureAsset.h"
#include "IAssetTransfer.h"

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

///@cond PRIVATE
struct EnumStr
{
    EnumStr(QString enumName, unsigned enumValue) :
        name(enumName),
        value(enumValue)
    {
    }
    
    EnumStr() :
        value(0xffffffff)
    {
    }

    QString name;
    unsigned value;
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

#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 7
EnumStr sceneBlendOps[] =
{
    EnumStr("add", Ogre::SBO_ADD),
    EnumStr("subtract", Ogre::SBO_SUBTRACT),
    EnumStr("reverse_subtract", Ogre::SBO_REVERSE_SUBTRACT),
    EnumStr("min", Ogre::SBO_MIN),
    EnumStr("max", Ogre::SBO_MAX),
    EnumStr()
};
#endif

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
    while (ptr && ptr->name.length() > 0)
    {
        if (ptr->name == name)
            return ptr->value;
        ++ptr;
    }
    
    return enums->value;
}
/// @endcond

OgreMaterialAsset::~OgreMaterialAsset()
{
    Unload();
}

namespace OgreRenderer
{
    extern QString lastLoadedOgreMaterial;
}

bool OgreMaterialAsset::DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous)
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

    std::string sanitatedname = AssetAPI::SanitateAssetRef(assetName);
    
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
                            line = "material " + AddDoubleQuotesIfNecessary(sanitatedname);
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
                            // Push the found texture reference to this materials dependencies.
                            std::string tex_name = QString(line.substr(8).c_str()).trimmed().toStdString();
                            QString absolute_tex_name = assetAPI->ResolveAssetRef(Name(), tex_name.c_str());
                            references_.push_back(AssetReference(absolute_tex_name));

                            // Ask the texture the internal name that should be used inside the material script.
                            absolute_tex_name = TextureAsset::NameInternal(absolute_tex_name);
                            line = "texture " + AddDoubleQuotesIfNecessary(AssetAPI::SanitateAssetRef(absolute_tex_name).toStdString());
                        }
                        // Check for shadow_caster_material reference
                        if (line.substr(0, 23) == "shadow_caster_material " && line.length() > 23)
                        {
                            std::string mat_name = QString(line.substr(23).c_str()).trimmed().toStdString();
                            QString absolute_mat_name = assetAPI->ResolveAssetRef(Name(), mat_name.c_str());
                            references_.push_back(AssetReference(absolute_mat_name));
                            // Sanitate the asset reference
                            line = "shadow_caster_material " + AddDoubleQuotesIfNecessary(AssetAPI::SanitateAssetRef(absolute_mat_name).toStdString());
                        }
                        // Check for cubic_texture reference
                        if (line.substr(0, 14) == "cubic_texture " && line.length() > 14)
                        {
                            // Ogre supports two formats for cubic_texture, but we ignore the short one.
                            // Format1 (short): cubic_texture <base_name> <combinedUVW|separateUV>
                            // Format2 (long): cubic_texture <front> <back> <left> <right> <up> <down> separateUV
                            QStringList textures = QString(line.substr(14).c_str()).split(" ", QString::SkipEmptyParts);
                            if (textures.size() == 2)
                            {
                                LogWarning("OgreMaterialAsset::DeserializeFromData: " + Name() + " - usage of \"cubic_texture <base_name> <combinedUVW|separateUV>\" detected. "
                                    "This format is not supported by the Asset API. Use \"cubic_texture <front> <back> <left> <right> <up> <down> separateUV\" instead.");
                            }
                            else if (textures.size() == 7)
                            {
                                std::string newLine = "cubic_texture";
                                for(int i = 0; i < textures.size() - 1; ++i) // ignore the separateUV param
                                {
                                    QString absoluteTexName = assetAPI->ResolveAssetRef(Name(), textures[i]);
                                    references_.push_back(AssetReference(absoluteTexName));
                                    // Sanitate the asset reference
                                    newLine += " " + AddDoubleQuotesIfNecessary(AssetAPI::SanitateAssetRef(absoluteTexName).toStdString());
                                }
                                // Put the last line as is
                                newLine += " " + textures.last().toStdString();
                                line = newLine;
                            }
                            else
                            {
                                LogWarning("OgreMaterialAsset::DeserializeFromData: malformed syntax for cubic_texture encountered. "
                                    "The supported format is \"cubic_texture <front> <back> <left> <right> <up> <down> separateUV\".");
                            }
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

        parsedOgreMaterialAsset = output.str();

        // HACK: The AssetAPI::AssetLoadCompleted mechanism cannot accommodate materials referring to other materials!
        // Everything loaded up?
        bool depsLoaded = true;
        for(size_t i = 0; i < references_.size(); ++i)
            if (!assetAPI->GetAsset(references_[i].ref))
            {
                depsLoaded = false;
                break;
            }

        if (depsLoaded)
        {
            CreateOgreMaterial(parsedOgreMaterialAsset);
            parsedOgreMaterialAsset = ""; // Save memory, this in-memory copy of the sanitated material is no longer used.
        }
    }
    catch(Ogre::Exception &e)
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

    assetAPI->AssetLoadCompleted(Name());
    return true;
}

void OgreMaterialAsset::DependencyLoaded(AssetPtr dependee)
{
    // Everything loaded up?
    for(size_t i = 0; i < references_.size(); ++i)
        if (!assetAPI->GetAsset(references_[i].ref))
            return;

    CreateOgreMaterial(parsedOgreMaterialAsset);
    parsedOgreMaterialAsset = ""; // Save memory, this in-memory copy of the sanitated material is no longer used.

    LoadCompleted();
}

AssetPtr OgreMaterialAsset::Clone(QString newAssetName) const
{
    assert(assetAPI);
    if (!IsLoaded())
        return AssetPtr();

    AssetPtr existing = assetAPI->GetAsset(newAssetName);
    if (existing)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": An asset with that name already exists!");
        return AssetPtr();
    }
    
    AssetPtr newAsset = assetAPI->CreateNewAsset(this->Type(), newAssetName);
    if (!newAsset)
    {
        LogError("Cannot Clone() asset \"" + Name() + "\" to a new asset \"" + newAssetName + "\": AssetAPI::CreateNewAsset failed!");
        return AssetPtr();
    }
    
    OgreMaterialAsset* destMat = dynamic_cast<OgreMaterialAsset*>(newAsset.get());
    assert(destMat);
    if (destMat)
        destMat->CopyContent((const_cast<OgreMaterialAsset*>(this))->shared_from_this());
    
    return newAsset;
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
        if (!CreateOgreMaterial())
            return;
    
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
    
    // Copy references
    references_ = sourceMat->references_;
}

void OgreMaterialAsset::CloneCompleted()
{
    assetAPI->AssetLoadCompleted(Name());
}

bool OgreMaterialAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    if (ogreMaterial.isNull())
    {
        LogWarning("SerializeTo: Tried to export non-existing Ogre material " + Name());
        return false;
    }
    try
    {
        Ogre::MaterialSerializer serializer;
        serializer.queueForExport(ogreMaterial);
        std::string materialData = serializer.getQueuedAsString();
        if (materialData.empty())
            return false;

        // Make sure that asset refs/IDs are desanitated.
        QStringList keywords;
        keywords << "material " << "texture " << "shadow_caster_material " << "cubix_texture ";
        OgreRenderer::DesanitateAssetIds(materialData, keywords);

        data.clear();
        data.insert(data.end(), &materialData[0], &materialData[0] + materialData.length());
        //serializer.exportQueued(filename);
    }
    catch(std::exception &e)
    {
        LogError("SerializeTo: Failed to export Ogre material " + Name() + ":");
        if (e.what())
            LogError(e.what());
        return false;
    }
    return true;
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

    // Remove spaces from start and end, otherwise 'texture = http://pic.png'
    // resolved the attribute to be ''
    QString keyTrimmedLower = key.trimmed().toLower();
    QStringList keyParts = keyTrimmedLower.split(' ');
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

Ogre::Technique* OgreMaterialAsset::GetTechnique(int techIndex) const
{
    if (ogreMaterial.isNull())
        return 0;
    if (techIndex < 0 || techIndex >= ogreMaterial->getNumTechniques())
        return 0;
    return ogreMaterial->getTechnique(techIndex);
}

Ogre::Pass* OgreMaterialAsset::GetPass(int techIndex, int passIndex) const
{
    Ogre::Technique* tech = GetTechnique(techIndex);
    if (!tech)
        return 0;
    if (passIndex < 0 || passIndex >= tech->getNumPasses())
        return 0;
    return tech->getPass(passIndex);
}

Ogre::TextureUnitState* OgreMaterialAsset::GetTextureUnit(int techIndex, int passIndex, int texUnitIndex) const
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

bool OgreMaterialAsset::CreateOgreMaterial(const std::string &materialData)
{
    Ogre::MaterialManager& matmgr = Ogre::MaterialManager::getSingleton(); 

#include "DisableMemoryLeakCheck.h"
    Ogre::DataStreamPtr modified_data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream((u8 *)(&materialData[0]), materialData.size()));
#include "EnableMemoryLeakCheck.h"

    OgreRenderer::lastLoadedOgreMaterial = this->Name();
    matmgr.parseScript(modified_data, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    OgreRenderer::lastLoadedOgreMaterial = "";
    QString assetName = AssetAPI::SanitateAssetRef(Name());
    ogreMaterial = matmgr.getByName(assetName.toStdString());
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
    
    Renderer::ShadowQualitySetting shadowQuality = Renderer::Shadows_High; ///\todo Regression. Read this ahead of time.

    //workaround: if receives shadows, check the amount of shadowmaps. If only 1 specified, add 2 more to support 3 shadowmaps
    if(ogreMaterial->getReceiveShadows() && shadowQuality == Renderer::Shadows_High && ogreMaterial->getNumTechniques() > 0)
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
                if (shadowmaps > 0 && shadowmaps < 3)
                {
                    Ogre::TextureUnitState* sm2 = pass->createTextureUnitState();
                    sm2->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);

                    Ogre::TextureUnitState* sm3 = pass->createTextureUnitState();
                    sm3->setContentType(Ogre::TextureUnitState::CONTENT_SHADOW);
                }
            }
        }
    }
    return true;
}

bool OgreMaterialAsset::CreateOgreMaterial()
{
    if (!ogreMaterial.isNull())
        return true;
    
    try
    {
        ogreAssetName = AssetAPI::SanitateAssetRef(Name());
        ogreMaterial = Ogre::MaterialManager::getSingleton().create(ogreAssetName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
    catch (Ogre::Exception& e)
    {
        LogError("OgreMaterialAsset: Failed to create empty material " + Name() + ", reason: " + e.what());
        return false;
    }
    
    return true;
}

int OgreMaterialAsset::CreateTechnique()
{
    if (ogreMaterial.isNull())
        if (!CreateOgreMaterial())
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
    if (ogreMaterial.isNull())
        if (!CreateOgreMaterial())
            return -1;
    
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
    if (ogreMaterial.isNull())
        if (!CreateOgreMaterial())
            return -1;
    
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
    size_t refIndex = 0;
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
    if (refIndex >= references_.size())
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
                LogError("SetTexture exception for " + Name() + ", reason: " + QString(e.what()));
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

QString OgreMaterialAsset::Texture(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::Texture: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return "";
    }

    return texUnit->getTextureName().c_str();
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

QString OgreMaterialAsset::VertexShader(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::VertexShader: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return "";
    }
    return pass->getVertexProgramName().c_str();
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

QString OgreMaterialAsset::PixelShader(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::PixelShader: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return "";
    }

    return pass->getFragmentProgramName().c_str();
}

bool OgreMaterialAsset::SetVertexShaderParameter(int techIndex, int passIndex, const QString& name, const QVariantList &value)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::SetVertexShaderParameter: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }
    const Ogre::GpuProgramPtr &verProg = pass->getVertexProgram();
    if (verProg.isNull())
    {
        LogError("OgreMaterialAsset::SetVertexShaderParameter: pass has no vertex program.");
        return false;
    }
    Ogre::GpuProgramParametersSharedPtr verPtr = pass->getVertexProgramParameters();
    if (!verPtr->hasNamedParameters())
    {
        LogError("OgreMaterialAsset::SetVertexShaderParameter: vertex program has no named parameters.");
        return false;
    }
    Ogre::GpuConstantDefinitionIterator mapIter = verPtr->getConstantDefinitionIterator();
    while(mapIter.hasMoreElements())
    {
        QString paramName(mapIter.peekNextKey().c_str());
        const Ogre::GpuConstantDefinition &paramDef = mapIter.getNext();
        if (paramName.lastIndexOf("[0]") != -1) // Filter names that end with '[0]'
            continue;
        if (paramName != name)
            continue;

        bool isFloat = paramDef.isFloat();
        size_t size = paramDef.elementSize * paramDef.arraySize;

        if (value.size() == 16)
        {
            LogDebug("writing float4x4");
            Ogre::Matrix4 matrix(
                value[0].toDouble(), value[1].toDouble(), value[2].toDouble(), value[3].toDouble(),
                value[4].toDouble(), value[5].toDouble(), value[6].toDouble(), value[7].toDouble(),
                value[8].toDouble(), value[9].toDouble(), value[10].toDouble(), value[11].toDouble(),
                value[12].toDouble(), value[13].toDouble(), value[14].toDouble(), value[15].toDouble());
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
            verPtr->_writeRawConstant(paramDef.physicalIndex, matrix);
#else
            verPtr->_writeRawConstant(paramDef.physicalIndex, matrix, size);
            return true;
#endif
        }
        else if (value.size() == 4)
        {
            Ogre::Vector4 vector;
            if (isFloat)
            {
                LogDebug("writing float4");
                vector = Ogre::Vector4(value[0].toDouble(), value[1].toDouble(), value[2].toDouble(), value[3].toDouble());
            }
            else
            {
                LogDebug("writing int4");
                vector = Ogre::Vector4(value[0].toInt(), value[1].toInt(), value[2].toInt(), value[3].toInt());
            }

            verPtr->_writeRawConstant(paramDef.physicalIndex, vector);
            return true;
        }
        else if (value.size() == 1)
        {
            if (isFloat)
            {
                LogDebug("Writing float");
                verPtr->_writeRawConstant(paramDef.physicalIndex, (Ogre::Real)value[0].toDouble());
            }
            else
            {
                LogDebug("Writing int.");
                verPtr->_writeRawConstant(paramDef.physicalIndex, value[0].toInt());
            }
            return true;
        }
        else
        {
            LogError(QString("OgreMaterialAsset::SetVertexShaderParameter: Invalid value count %1 for %2: %3 expected.").arg(value.size()).arg(name).arg(size));
            return false;
        }
    }

    return false;
}

bool OgreMaterialAsset::SetPixelShaderParameter(int techIndex, int passIndex, const QString& name, const QVariantList &value)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::SetPixelShaderParameter: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }
    const Ogre::GpuProgramPtr &fragProg = pass->getFragmentProgram();
    if (fragProg.isNull())
    {
        LogError("OgreMaterialAsset::SetPixelShaderParameter: pass has no fragment program.");
        return false;
    }
    Ogre::GpuProgramParametersSharedPtr fragPtr = pass->getFragmentProgramParameters();
    if (!fragPtr->hasNamedParameters())
    {
        LogError("OgreMaterialAsset::SetPixelShaderParameter: fragment program has no named parameters.");
        return false;
    }
    Ogre::GpuConstantDefinitionIterator mapIter = fragPtr->getConstantDefinitionIterator();
    while(mapIter.hasMoreElements())
    {
        QString paramName(mapIter.peekNextKey().c_str());
        const Ogre::GpuConstantDefinition &paramDef = mapIter.getNext();
        if (paramName.lastIndexOf("[0]") != -1) // Filter names that end with '[0]'
            continue;
        if (paramName != name)
            continue;

        bool isFloat = paramDef.isFloat();
        size_t size = paramDef.elementSize * paramDef.arraySize;

        if (value.size() == 16)
        {
            LogDebug("writing float4x4");
            Ogre::Matrix4 matrix(
                value[0].toDouble(), value[1].toDouble(), value[2].toDouble(), value[3].toDouble(),
                value[4].toDouble(), value[5].toDouble(), value[6].toDouble(), value[7].toDouble(),
                value[8].toDouble(), value[9].toDouble(), value[10].toDouble(), value[11].toDouble(),
                value[12].toDouble(), value[13].toDouble(), value[14].toDouble(), value[15].toDouble());
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
            fragPtr->_writeRawConstant(paramDef.physicalIndex, matrix);
#else
            fragPtr->_writeRawConstant(paramDef.physicalIndex, matrix, size);
            return true;
#endif
        }
        else if (value.size() == 4)
        {
            Ogre::Vector4 vector;
            if (isFloat)
            {
                LogDebug("writing float4");
                vector = Ogre::Vector4(value[0].toDouble(), value[1].toDouble(), value[2].toDouble(), value[3].toDouble());
            }
            else
            {
                LogDebug("writing int4");
                vector = Ogre::Vector4(value[0].toInt(), value[1].toInt(), value[2].toInt(), value[3].toInt());
            }

            fragPtr->_writeRawConstant(paramDef.physicalIndex, vector);
            return true;
        }
        else if (value.size() == 1)
        {
            if (isFloat)
            {
                LogDebug("Writing float");
                fragPtr->_writeRawConstant(paramDef.physicalIndex, (Ogre::Real)value[0].toDouble());
            }
            else
            {
                LogDebug("Writing int.");
                fragPtr->_writeRawConstant(paramDef.physicalIndex, value[0].toInt());
            }
            return true;
        }
        else
        {
            LogError(QString("OgreMaterialAsset::SetPixelShaderParameter: Invalid value count %1 for %2: %3 expected.").arg(value.size()).arg(name).arg(size));
            return false;
        }
    }

    return false;
}

bool OgreMaterialAsset::SetLighting(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setLightingEnabled(enable);
    return true;
}

bool OgreMaterialAsset::IsLightingEnabled(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::IsLightingEnabled: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getLightingEnabled();
}

bool OgreMaterialAsset::SetDiffuseColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDiffuse(color);
    return true;
}

Color OgreMaterialAsset::DiffuseColor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::DiffuseColor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return Color();
    }

    return pass->getDiffuse();
}

bool OgreMaterialAsset::SetAmbientColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setAmbient(color);
    return true;
}

Color OgreMaterialAsset::AmbientColor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::AmbientColor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return Color();
    }

    return pass->getAmbient();
}

bool OgreMaterialAsset::SetSpecularColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSpecular(color);
    return true;
}

Color OgreMaterialAsset::SpecularColor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::SpecularColor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return Color();
    }

    return pass->getSpecular();
}

bool OgreMaterialAsset::SetEmissiveColor(int techIndex, int passIndex, const Color& color)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSelfIllumination(color);
    return true;
}

Color OgreMaterialAsset::EmissiveColor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::EmissiveColor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return Color();
    }

    return pass->getSelfIllumination();
}

bool OgreMaterialAsset::SetSceneBlend(int techIndex, int passIndex, unsigned blendMode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending((Ogre::SceneBlendType)blendMode);
    return true;
}

bool OgreMaterialAsset::SetSceneBlend(int techIndex, int passIndex, unsigned srcFactor, unsigned dstFactor)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setSceneBlending((Ogre::SceneBlendFactor)srcFactor, (Ogre::SceneBlendFactor)dstFactor);
    return true;
}

unsigned OgreMaterialAsset::SourceSceneBlendFactor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::SourceSceneBlendFactor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return 0;
    }

    return pass->getSourceBlendFactor();
}

unsigned OgreMaterialAsset::DestinationSceneBlendFactor(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::DestinationSceneBlendFactor: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return 0;
    }

    return pass->getDestBlendFactor();
}

bool OgreMaterialAsset::SetPolygonMode(int techIndex, int passIndex, unsigned polygonMode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setPolygonMode((Ogre::PolygonMode)polygonMode);
    return true;
}

unsigned OgreMaterialAsset::PolygonMode(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::PolygonMode: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return 0;
    }

    return pass->getPolygonMode();
}

bool OgreMaterialAsset::SetDepthCheck(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthCheckEnabled(enable);
    return true;
}

bool OgreMaterialAsset::IsDepthCheckEnabled(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::IsDepthCheckEnabled: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getDepthCheckEnabled();
}

bool OgreMaterialAsset::SetDepthWrite(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthWriteEnabled(enable);
    return true;
}

bool OgreMaterialAsset::IsDepthWriteEnabled(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::IsDepthWriteEnabled: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getDepthWriteEnabled();
}

bool OgreMaterialAsset::SetDepthBias(int techIndex, int passIndex, float bias)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setDepthBias(bias);
    return true;
}

float OgreMaterialAsset::DepthBias(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
      LogError(QString("OgreMaterialAsset::DepthBias: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return 0.f;
    }

    return pass->getDepthBiasConstant();
}


bool OgreMaterialAsset::SetHardwareCullingMode(int techIndex, int passIndex, unsigned mode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setCullingMode((Ogre::CullingMode)mode);
    return true;
}

unsigned OgreMaterialAsset::HardwareCullingMode(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::HardwareCullingMode: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getCullingMode();
}

bool OgreMaterialAsset::SetShadingMode(int techIndex, int passIndex, unsigned mode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setShadingMode((Ogre::ShadeOptions)mode);
    return true;
}

unsigned OgreMaterialAsset::ShadingMode(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::ShadingMode: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getShadingMode();
}

bool OgreMaterialAsset::SetFillMode(int techIndex, int passIndex, unsigned mode)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setPolygonMode((Ogre::PolygonMode)mode);
    return true;
}

unsigned OgreMaterialAsset::FillMode(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::FillMode: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getPolygonMode();
}

bool OgreMaterialAsset::SetColorWrite(int techIndex, int passIndex, bool enable)
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
        return false;
    pass->setColourWriteEnabled(enable);
    return true;
}

bool OgreMaterialAsset::IsColorWriteEnabled(int techIndex, int passIndex) const
{
    Ogre::Pass* pass = GetPass(techIndex, passIndex);
    if (!pass)
    {
        LogError(QString("OgreMaterialAsset::IsColorWriteEnabled: Could not find technique %1 pass %2.").arg(techIndex).arg(passIndex));
        return false;
    }

    return pass->getColourWriteEnabled();
}

bool OgreMaterialAsset::SetTextureCoordSet(int techIndex, int passIndex, int texUnitIndex, uint value)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    texUnit->setTextureCoordSet(value);
    return true;
}

uint OgreMaterialAsset::TextureCoordSet(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::TextureCoordSet: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0;
    }
    return texUnit->getTextureCoordSet();
}

bool OgreMaterialAsset::SetTextureAddressingMode(int techIndex, int passIndex, int texUnitIndex, unsigned mode)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    texUnit->setTextureAddressingMode((Ogre::TextureUnitState::TextureAddressingMode)mode);
    return true;
}

bool OgreMaterialAsset::SetTextureAddressingMode(int techIndex, int passIndex, int texUnitIndex, unsigned uMode, unsigned vMode, unsigned wMode)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    texUnit->setTextureAddressingMode(
        (Ogre::TextureUnitState::TextureAddressingMode)uMode,
        (Ogre::TextureUnitState::TextureAddressingMode)vMode,
        (Ogre::TextureUnitState::TextureAddressingMode)wMode);
    return true;
}

unsigned OgreMaterialAsset::TextureAddressingModeU(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::TextureAddressingModeU: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0;
    }
    return texUnit->getTextureAddressingMode().u;
}

unsigned OgreMaterialAsset::TextureAddressingModeV(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::TextureAddressingModeV: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0;
    }
    return texUnit->getTextureAddressingMode().v;
}

unsigned OgreMaterialAsset::TextureAddressingModeW(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::TextureAddressingModeW: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0;
    }
    return texUnit->getTextureAddressingMode().w;
}

bool OgreMaterialAsset::SetScrollAnimation(int techIndex, int passIndex, int texUnitIndex, float uSpeed, float vSpeed)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    texUnit->setScrollAnimation(uSpeed, vSpeed);
    return true;
}

float OgreMaterialAsset::ScrollAnimationU(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::ScrollAnimationU: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0.f;
    }

    const Ogre::TextureUnitState::EffectMap &effects = texUnit->getEffects();
    Ogre::TextureUnitState::EffectMap::const_iterator it = effects.find(Ogre::TextureUnitState::ET_USCROLL);
    if (it == effects.end())
        it = effects.find(Ogre::TextureUnitState::ET_UVSCROLL);
    if (it == effects.end())
    {
         LogError("OgreMaterialAsset::ScrollAnimationU: Could not find Ogre::TextureUnitState::ET_USCROLL or ET_UVSCROLL from effects map.");
         return 0.f;
    }

    return (*it).second.arg1;
}

float OgreMaterialAsset::ScrollAnimationV(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::ScrollAnimationV: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0.f;
    }

    const Ogre::TextureUnitState::EffectMap &effects = texUnit->getEffects();
    Ogre::TextureUnitState::EffectMap::const_iterator it = effects.find(Ogre::TextureUnitState::ET_VSCROLL);
    if (it == effects.end())
        it = effects.find(Ogre::TextureUnitState::ET_UVSCROLL);
    if (it == effects.end())
    {
         LogError("OgreMaterialAsset::ScrollAnimationV: Could not find Ogre::TextureUnitState::ET_VSCROLL or ET_UVSCROLL from effects map.");
         return 0.f;
    }

    return (*it).second.arg1;
}

bool OgreMaterialAsset::SetRotateAnimation(int techIndex, int passIndex, int texUnitIndex, float speed)
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
        return false;
    texUnit->setRotateAnimation(speed);
    return true;
}

float OgreMaterialAsset::RotateAnimation(int techIndex, int passIndex, int texUnitIndex) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::RotateAnimation: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        return 0.f;
    }

    const Ogre::TextureUnitState::EffectMap &effects = texUnit->getEffects();
    Ogre::TextureUnitState::EffectMap::const_iterator it = texUnit->getEffects().find(Ogre::TextureUnitState::ET_ROTATE);
    if (it == effects.end())
    {
         LogError("OgreMaterialAsset::RotateAnimation: Could not find Ogre::TextureUnitState::ET_ROTATE from effects map.");
         return 0.f;
    }

    return (*it).second.arg1;
}

bool OgreMaterialAsset::HasTextureEffect(int techIndex, int passIndex, int texUnitIndex, unsigned effect) const
{
    Ogre::TextureUnitState* texUnit = GetTextureUnit(techIndex, passIndex, texUnitIndex);
    if (!texUnit)
    {
        LogError(QString("OgreMaterialAsset::HasTextureEffect: Could not find techique %1 pass %2 texture unit %3.").arg(techIndex).arg(passIndex).arg(texUnitIndex));
        false;
    }

    const Ogre::TextureUnitState::EffectMap &effects = texUnit->getEffects();
    Ogre::TextureUnitState::EffectMap::const_iterator it = effects.find((Ogre::TextureUnitState::TextureEffectType)effect);
    return it != effects.end();
}

bool OgreMaterialAsset::SetMaterialAttribute(const QString& attr, const QString& val, const QString& origVal)
{
    if (attr == "receive_shadows")
    {
        ogreMaterial->setReceiveShadows(ParseBool(val));
        return true;
    }
    if (attr == "transparency_casts_shadows")
    {
        ogreMaterial->setTransparencyCastsShadows(ParseBool(val));
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
        SetAmbientColor(techIndex, passIndex, Color::FromString(val));
        return true;
    }
    if (attr == "diffuse")
    {
        SetDiffuseColor(techIndex, passIndex, Color::FromString(val));
        return true;
    }
    if (attr == "specular")
    {
        SetSpecularColor(techIndex, passIndex, Color::FromString(val));
        return true;
    }
    if (attr == "emissive")
    {
        SetEmissiveColor(techIndex, passIndex, Color::FromString(val));
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
#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 7
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
#endif
    if (attr == "depth_check")
    {
        SetDepthCheck(techIndex, passIndex, ParseBool(val));
        return true;
    }
    if (attr == "depth_write")
    {
        SetDepthWrite(techIndex, passIndex, ParseBool(val));
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
        pass->setNormaliseNormals(ParseBool(val));
        return true;
    }
    if (attr == "transparent_sorting")
    {
        pass->setTransparentSortingEnabled(ParseBool(val));
        return true;
    }
    if (attr == "cull_hardware")
    {
        pass->setCullingMode((Ogre::CullingMode)GetEnumValue(val, cullingModes));
        return true;
    }
    if (attr == "lighting")
    {
        SetLighting(techIndex, passIndex, ParseBool(val));
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
        pass->setColourWriteEnabled(ParseBool(val));
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
        texUnit->setTextureBorderColour(Color::FromString(val));
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
    for (size_t i = pendingApplies.size() - 1; i < pendingApplies.size(); --i)
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
                        LogError("SetTexture exception for " + Name() + ", reason: " + QString(e.what()));
                    }
                }
            }
            pendingApplies.erase(pendingApplies.begin() + i);
        }
    }
}

void OgreMaterialAsset::OnTransferFailed(IAssetTransfer* transfer, QString reason)
{
    for (size_t i = pendingApplies.size() - 1; i < pendingApplies.size(); --i)
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
                    LogError("SetTexture exception for " + Name() + ", reason: " + QString(e.what()));
                }
            }
            pendingApplies.erase(pendingApplies.begin() + i);
        }
    }
}
