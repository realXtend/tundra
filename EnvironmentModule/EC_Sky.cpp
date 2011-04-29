// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_Sky.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "TextureAsset.h"
#include "AttributeMetadata.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

const unsigned int cSkyBoxTextureCount = 6; ///< Sky box has 6 textures.

namespace Environment
{

/// \todo Use Asset API for fetching sky resources.
EC_Sky::EC_Sky(IModule *module) :
    IComponent(module->GetFramework()),
    materialRef(this, "Material", AssetReference("RexSkyBox")), ///< \todo Add "orge://" when AssetAPI can handle it.
    textureRefs(this, "Texture"),
    orientation(this, "Orientation", Quaternion(f32(M_PI/2.0), Vector3df(1.0,0.0,0.0))),
    distance(this, "Distance",50.0),
    drawFirst(this, "Draw first", true)
{
     connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));

     static AttributeMetadata materialRefMetadata;
     AttributeMetadata::ButtonInfoList materialRefButtons;
     materialRefButtons.push_back(AttributeMetadata::ButtonInfo(materialRef.GetName(), "V", "View"));
     materialRefMetadata.buttons = materialRefButtons;
     materialRef.SetMetadata(&materialRefMetadata);

     // Find out default textures.
     renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();

     StringVector names;
     Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
     if (materialPtr.get() != 0)
     {
         OgreRenderer::GetTextureNamesFromMaterial(materialPtr, names);
         AssetReferenceList lst;
         if (names.size() == cSkyBoxTextureCount)
         {
            // This code block is not currently working, but if for some reason GetTextureNamesFromMaterial understands cubic_textures this codeblock is runned
            for(int i = 0; i < cSkyBoxTextureCount; ++i)
                lst.Append(AssetReference(names[i].c_str()));
         }
         else
         {
            // Add default values, hardcoded
            /// HACK use hardcoded-values because ogre textureunit state class cannot find out texture names for cubic_texture type.
            lst.Append(AssetReference(names[0].c_str()));
            lst.Append(AssetReference("rex_sky_back.dds"));
            lst.Append(AssetReference("rex_sky_left.dds"));
            lst.Append(AssetReference("rex_sky_right.dds"));
            lst.Append(AssetReference("rex_sky_top.dds"));
            lst.Append(AssetReference("rex_sky_bot.dds"));
         }

        textureRefs.Set(lst, AttributeChange::LocalOnly);
     }

    // Disable old sky.
    // DisableSky();
    CreateSky();

    lastMaterial_ = materialRef.Get().ref;
    lastOrientation_ = orientation.Get();
    lastDistance_ = distance.Get();
    lastDrawFirst_ = drawFirst.Get();

    while(textureAssets.size() < cSkyBoxTextureCount)
        textureAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));


    for(int i = 0; i < cSkyBoxTextureCount; ++i)
        {
            connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            //materialAssets[i]->HandleAssetRefChange(framework_->Asset(), materials[i].ref);
        }
   
}

EC_Sky::~EC_Sky()
{
    DisableSky();
    renderer_.reset();
     
   while(textureAssets.size() > cSkyBoxTextureCount)
        textureAssets.pop_back();
}

void EC_Sky::CreateSky()
{
    if (!ViewEnabled())
        return;

    if (renderer_.expired())
        return;

    QString currentMaterial = materialRef.Get().ref;

    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
    //Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(framework_->Asset()->LookupAssetRefToStorage(materialRef.Get().ref).toStdString().c_str());
    if (materialPtr.isNull())
    {
        LogError("Could not get SkyBox material : " + currentMaterial.toStdString());
        return;
    }

    materialPtr->setReceiveShadows(false);

    try
    {
        //Vector3df v = angleAxisAttr.Get();
        //Ogre::Quaternion rotation(Ogre::Degree(90.0), Ogre::Vector3(1, 0, 0));
        Quaternion o = orientation.Get();
        renderer_.lock()->GetSceneManager()->setSkyBox(true, currentMaterial.toStdString().c_str(), distance.Get(),
            drawFirst.Get(), Ogre::Quaternion(o.w, o.x, o.y, o.z));
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not set SkyBox: " + std::string(e.what()));
    }
}

void EC_Sky::View(const QString &attributeName)
{
    /// @todo implement this.
}

void EC_Sky::OnTextureAssetLoaded(AssetPtr tex)
{
    std::vector<std::string> texture_names;
    texture_names.reserve(cSkyBoxTextureCount);
    
    AssetReferenceList textureList = textureRefs.Get();

    const char * const defaultSkyTextures[cSkyBoxTextureCount] =
    {   "rex_sky_front.dds",
        "rex_sky_back.dds",
        "rex_sky_left.dds",
        "rex_sky_right.dds",
        "rex_sky_top.dds",
        "rex_sky_bot.dds"
    };

    for(size_t i = 0; i < textureAssets.size() || i < cSkyBoxTextureCount; ++i)
        if (i < textureAssets.size() && textureAssets[i])
        {
            AssetPtr asset = textureAssets[i]->Asset();
            TextureAsset *textureAsset = dynamic_cast<TextureAsset*>(asset.get());
            if (textureAsset)
                texture_names.push_back(textureAsset->ogreAssetName.toStdString());
            else
                texture_names.push_back(defaultSkyTextures[i]);
        }
        else
            texture_names.push_back(defaultSkyTextures[i]);

    assert(texture_names.size() == cSkyBoxTextureCount);

    ///\todo Use AssetAPI for the material.
    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
    if (materialPtr.isNull())
    {
        LogError("EC_Sky::OnTextureAssetLoaded: Cannot find Ogre material \"" + materialRef.Get().ref.toStdString() + "\"!");
        return;
    }
    if (materialPtr->getNumTechniques() == 0 || materialPtr->getTechnique(0) == 0 ||
        materialPtr->getTechnique(0)->getNumPasses() == 0 || materialPtr->getTechnique(0)->getPass(0) == 0 ||
        materialPtr->getTechnique(0)->getPass(0)->getNumTextureUnitStates() == 0 ||
        materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0) == 0)
    {
        LogError("EC_Sky::OnTextureAssetLoaded: Cannot use material \"" + materialRef.Get().ref.toStdString() + "\" as Skybox material: It has 0 techniques, passes or texture unit states!");
        return;
    }

    materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&texture_names[0], false);
    
    CreateSky();    
}

void EC_Sky::OnAttributeUpdated(IAttribute* attribute)
{
    if (!ViewEnabled())
        return;

    std::string name = attribute->GetNameString();
    if ((name == materialRef.GetNameString() && materialRef.Get().ref != lastMaterial_ ) ||
        (name ==  distance.GetNameString() && distance.Get() != lastDistance_ ) ||
        (name == drawFirst.GetNameString() && drawFirst.Get() != lastDrawFirst_ ))
    {
        DisableSky();
        CreateSky();

        lastMaterial_ = materialRef.Get().ref;
        lastDistance_ = distance.Get();
        lastDrawFirst_ = drawFirst.Get();
    }
    else if (name == textureRefs.GetNameString() )
    {
      
        AssetReferenceList textures = textureRefs.Get();

        // Reallocate the number of texture asset reflisteners.
        while(textureAssets.size() > (size_t)textures.Size())
            textureAssets.pop_back();
        while(textureAssets.size() < (size_t)textures.Size())
            textureAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));

        for(int i = 0; i < textures.Size(); ++i)
        {
            connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            textureAssets[i]->HandleAssetRefChange(framework_->Asset(), textures[i].ref);
        }

     
        //SetTextures();
    }
}

void EC_Sky::SetTextures()
{
    // Depricated..
    if (!ViewEnabled())
        return;

    AssetReferenceList lst = textureRefs.Get();
    std::vector<std::string> texture_names;
    texture_names.reserve(cSkyBoxTextureCount);

    for(int i = 0; i < lst.Size() && i <= cSkyBoxTextureCount; ++i)
        texture_names.push_back(lst[i].ref.toStdString());

    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
    if (!materialPtr.isNull() && texture_names.size() == cSkyBoxTextureCount)
        materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&texture_names[0], false);
        //skyMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureScale(1, -1);
    else if(!materialPtr.isNull() )
        for(size_t i = 0; i < texture_names.size(); ++i)
            materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setFrameTextureName(Ogre::String(texture_names[i].c_str()), i);

    DisableSky();
    CreateSky();
}

void EC_Sky::DisableSky()
{
    if (!ViewEnabled())
        return;

    if (!renderer_.expired())
        renderer_.lock()->GetSceneManager()->setSkyBox(false, "");
}

}
