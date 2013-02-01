// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Sky.h"
#include "Scene/Scene.h"
#include "OgreWorld.h"
#include "LoggingFunctions.h"
#include "TextureAsset.h"
#include "OgreMaterialAsset.h"
#include "IAssetTransfer.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

namespace
{

const unsigned int cSkyBoxTextureCount = 6;
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
    materialRef(this, "Material", AssetReference("local://RexSkyBox.material", "OgreMaterial")), /**< @todo Shouldn't we be able to use "Ogre Media:RexSkyBox.material"? Doens't seem to work. */
    textureRefs(this, "Texture", AssetReferenceList("Texture")),
    orientation(this, "Orientation", Quat::identity),
    distance(this, "Distance", 999.0), /**< @todo 5000 is the Ogre's default value, but for some reason value geater than 999 makes the sky box black. */
    drawFirst(this, "Draw first", true),
    enabled(true)
//    enabled(this, "Enabled", true)
{
    if (scene)
        ogreWorld = scene->GetWorld<OgreWorld>();

    materialAsset = MAKE_SHARED(AssetRefListener);
    connect(materialAsset.get(), SIGNAL(Loaded(AssetPtr)), SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
}

EC_Sky::~EC_Sky()
{
    enabled = false;
    Update();
//    enabled.Set(false, AttributeChange::LocalOnly);

    while(textureAssets.size() > cSkyBoxTextureCount)
        textureAssets.pop_back();
}

void EC_Sky::AttributesChanged()
{
    if (!ViewEnabled())
        return;

    if (materialRef.ValueChanged())
    {
        /// @todo Remove the following check at some point.
        if (!materialRef.Get().ref.endsWith(".material", Qt::CaseInsensitive))
        {
            LogWarning("EC_Sky::OnAttributeUpdated: defining material by using Ogre resource name "
                "will be deprecated. Use the actual material asset name instead.");
            currentMaterial = materialRef.Get().ref;
            Update();
        }
        else
        {
            materialAsset->HandleAssetRefChange(framework->Asset(), materialRef.Get().ref);
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
            textureAssets.push_back(MAKE_SHARED(AssetRefListener));

        for(int i = 0; i < textures.Size(); ++i)
        {
            connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            textureAssets[i]->HandleAssetRefChange(framework->Asset(), textures[i].ref);
        }
    }

    if (distance.ValueChanged() || drawFirst.ValueChanged() || orientation.ValueChanged() /*|| enabled.ValueChanged()*/)
        Update();
}

void EC_Sky::Update()
{
    if (!ViewEnabled() || ogreWorld.expired())
        return;

    try
    {
        if (!enabled || (enabled/*.Get()*/ && !currentMaterial.isEmpty())) // If enabled == true, do not allow passing empty material name (material not loaded yet).
            ogreWorld.lock()->OgreSceneManager()->setSkyBox(enabled/*.Get()*/, currentMaterial.toStdString(), distance.Get(), drawFirst.Get(), orientation.Get());
    }
    catch(const Ogre::Exception &e)
    {
        LogError("EC_Sky::Update: Could not set sky box " + BoolToString(enabled/*.Get()*/) + ": " + QString(e.what()));
    }
}

void EC_Sky::OnMaterialAssetLoaded(AssetPtr mat)
{
    OgreMaterialAssetPtr material = dynamic_pointer_cast<OgreMaterialAsset>(mat);
    if (!material)
        return;
    if (material->ogreMaterial.isNull())
        return;

    currentMaterial = material->ogreAssetName;
    /// @todo Remove the following line? It's should be up to the user/artist to author materials properly for the use.
    material->ogreMaterial->setReceiveShadows(false);
    Update();
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

    Update();
}
