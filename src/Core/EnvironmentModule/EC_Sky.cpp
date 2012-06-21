// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Sky.h"
#include "Renderer.h"
#include "Scene.h"
#include "OgreMaterialUtils.h"
#include "OgreWorld.h"
#include "LoggingFunctions.h"
#include "TextureAsset.h"
#include "OgreMaterialAsset.h"
#include "IAssetTransfer.h"

#include <Ogre.h>

#include <boost/make_shared.hpp>

#include "MemoryLeakCheck.h"

namespace
{

const unsigned int cSkyBoxTextureCount = 6;
const char * const cDefaultSkyBoxMaterial = "Ogre Media:RexSkyBox.material";
const char * const cDeprecatedMaterialName = "RexSkyBox";
const char * const cDefaultSkyBoxTextures[cSkyBoxTextureCount] =
{
    "rex_sky_front.dds",
    "rex_sky_back.dds",
    "rex_sky_left.dds",
    "rex_sky_right.dds",
    "rex_sky_top.dds",
    "rex_sky_bot.dds"
};

} // ~unnamed namespace

EC_Sky::EC_Sky(Scene* scene) :
    IComponent(scene),
    materialRef(this, "Material", AssetReference(cDefaultSkyBoxMaterial)),
    textureRefs(this, "Texture", AssetReferenceList("Texture")),
    orientation(this, "Orientation", Quat::identity),
    distance(this, "Distance", 999.0), /**< @todo 5000 is the Ogre's default value, but for some reason value geater than 999 makes the sky box black. */
    drawFirst(this, "Draw first", true)
{
    if (scene)
        ogreWorld = scene->GetWorld<OgreWorld>();

/*
    // Find out default textures.
    StringVector names;
    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
    if (materialPtr.get() != 0)
    {
        OgreRenderer::GetTextureNamesFromMaterial(materialPtr, names);
        AssetReferenceList lst("Texture");
        if (names.size() == cSkyBoxTextureCount)
        {
            // This code block is not currently working, but if for some reason GetTextureNamesFromMaterial understands cubic_textures this codeblock is runned
            for(unsigned int i = 0; i < cSkyBoxTextureCount; ++i)
                lst.Append(AssetReference(names[i]));
        }
        else
        {
            // Add default values, hardcoded
            /// HACK use hardcoded-values because ogre textureunit state class cannot find out texture names for cubic_texture type.
            lst.Append(AssetReference(cDefaultSkyBoxTextures[0]));
            lst.Append(AssetReference(cDefaultSkyBoxTextures[1]));
            lst.Append(AssetReference(cDefaultSkyBoxTextures[2]));
            lst.Append(AssetReference(cDefaultSkyBoxTextures[3]));
            lst.Append(AssetReference(cDefaultSkyBoxTextures[4]));
            lst.Append(AssetReference(cDefaultSkyBoxTextures[5]));
        }

        textureRefs.Set(lst, AttributeChange::LocalOnly);
    }

    while(textureAssets.size() < cSkyBoxTextureCount)
        textureAssets.push_back(boost::make_shared<AssetRefListener>());

    for(unsigned int i = 0; i < cSkyBoxTextureCount; ++i)
    {
        connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
        //materialAssets[i]->HandleAssetRefChange(framework->Asset(), materials[i].ref);
    }
*/
}

EC_Sky::~EC_Sky()
{
    DisableSky();

    while(textureAssets.size() > cSkyBoxTextureCount)
        textureAssets.pop_back();
}

void EC_Sky::EnableSky()
{
    try
    {
        if (!ViewEnabled() && !ogreWorld.expired() && !currentMaterial.isEmpty())
            ogreWorld.lock()->OgreSceneManager()->setSkyBox(true, currentMaterial.toStdString(), distance.Get(), drawFirst.Get(), orientation.Get());
    }
    catch(const Ogre::Exception &e)
    {
        LogError("EC_Sky::EnableSky: Could not set sky box: " + std::string(e.what()));
    }
}

void EC_Sky::DisableSky()
{
    if (ViewEnabled() && !ogreWorld.expired())
        ogreWorld.lock()->OgreSceneManager()->setSkyBox(false, "");
}

void EC_Sky::AttributesChanged()
{
    if (!ViewEnabled())
        return;

    if (materialRef.ValueChanged())
    {
        if (!materialRef.Get().ref.endsWith(".material", Qt::CaseInsensitive))
        {
            LogWarning("EC_Sky::OnAttributeUpdated: defining material by using Ogre resource name "
                "will be deprecated. Use the actual material asset name instead.");
            currentMaterial = materialRef.Get().ref;
        }
        else
        {
            AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(materialRef.Get());
            if (transfer)
                connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            // Completion of the material asset transfer will call EnableSky
        }
    }

    if (textureRefs.ValueChanged() && !textureRefs.Get().IsEmpty())
    {
         // Usage of textureRefs attribute is deprecated. But for now, print warning and proceed with texture retrieval.
        LogWarning("EC_Sky::OnAttributeUpdated: Settings sky textures separately is deprecated and "
            "will be removed in the future. Use the materialRef attribute instead, and set textureRefs to be empty.");

        const AssetReferenceList &textures = textureRefs.Get();
        // Reallocate the number of texture asset reflisteners.
        while(textureAssets.size() > (size_t)textures.Size())
            textureAssets.pop_back();
        while(textureAssets.size() < (size_t)textures.Size())
            textureAssets.push_back(boost::make_shared<AssetRefListener>());

        for(int i = 0; i < textures.Size(); ++i)
        {
            connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            textureAssets[i]->HandleAssetRefChange(framework->Asset(), textures[i].ref);
        }
    }

    if (distance.ValueChanged() || drawFirst.ValueChanged() || orientation.ValueChanged())
        EnableSky();
}

void EC_Sky::OnMaterialAssetLoaded(AssetPtr mat)
{
    OgreMaterialAssetPtr ogreMaterial = boost::dynamic_pointer_cast<OgreMaterialAsset>(mat);
    assert(ogreMaterial);
    if (!ogreMaterial)
        return;

    currentMaterial = ogreMaterial->ogreAssetName;
    ogreMaterial->ogreMaterial->setReceiveShadows(false);
/*
    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString());
    //Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(framework->Asset()->LookupAssetRefToStorage(materialRef.Get().ref).toStdString().c_str());
    if (materialPtr.isNull())
    {
        LogError("EC_Sky::EnableSky: Could not get sky box material: " + materialRef.Get().ref);
        return;
    }
*/
    EnableSky();
}

void EC_Sky::OnTextureAssetLoaded(AssetPtr tex)
{
    std::vector<std::string> textureNames;
    textureNames.reserve(cSkyBoxTextureCount);

    for(size_t i = 0; i < textureAssets.size() || i < cSkyBoxTextureCount; ++i)
        if (i < textureAssets.size() && textureAssets[i])
        {
            AssetPtr asset = textureAssets[i]->Asset();
            TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(asset.get());
            if (textureAsset)
                textureNames.push_back(textureAsset->ogreAssetName.toStdString());
            else
                textureNames.push_back(cDefaultSkyBoxTextures[i]);
        }
        else
            textureNames.push_back(cDefaultSkyBoxTextures[i]);

    assert(textureNames.size() == cSkyBoxTextureCount);

    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString());
    if (materialPtr.isNull())
    {
        LogError("EC_Sky::OnTextureAssetLoaded: Cannot find Ogre material \"" + currentMaterial + "\"!");
        return;
    }
    if (materialPtr->getNumTechniques() == 0 || materialPtr->getTechnique(0) == 0 ||
        materialPtr->getTechnique(0)->getNumPasses() == 0 || materialPtr->getTechnique(0)->getPass(0) == 0 ||
        materialPtr->getTechnique(0)->getPass(0)->getNumTextureUnitStates() == 0 ||
        materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0) == 0)
    {
        LogError("EC_Sky::OnTextureAssetLoaded: Cannot use material \"" + currentMaterial +
            "\" as Skybox material: It has 0 techniques, passes or texture unit states!");
        return;
    }

    materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&textureNames[0], false);

    EnableSky();
}
