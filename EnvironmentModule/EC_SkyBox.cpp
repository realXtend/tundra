// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_SkyBox.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "TextureAsset.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_SkyBox")

#include <Ogre.h>

#include "MemoryLeakCheck.h"

const unsigned int cSkyBoxTextureCount = 6; ///< Sky box has 6 textures.

namespace Environment
{

/// \todo Use Asset API for fetching sky resources.
EC_SkyBox::EC_SkyBox(IModule *module) :
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
     if ( materialPtr.get() != 0)
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

EC_SkyBox::~EC_SkyBox()
{
    DisableSky();
    renderer_.reset();
     
   while(textureAssets.size() > cSkyBoxTextureCount)
        textureAssets.pop_back();
}

void EC_SkyBox::CreateSky()
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
        //RexTypes::Vector3 v = angleAxisAttr.Get();
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

void EC_SkyBox::View(const QString &attributeName)
{
    /// @todo implement this.
}

void EC_SkyBox::OnTextureAssetLoaded(AssetPtr tex)
{
    TextureAsset* texture = dynamic_cast<TextureAsset* >(tex.get());
   
    std::vector<std::string> texture_names;
    texture_names.reserve(cSkyBoxTextureCount);
    bool assetUsed = false;
    
    AssetReferenceList textureList = textureRefs.Get();
    
    for(int i = 0; i < textureList.Size(); ++i)
        if (textureList[i].ref == texture->Name() ||
            framework_->Asset()->LookupAssetRefToStorage(textureList[i].ref) == texture->Name()) ///<///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        {
         
            texture_names.push_back(texture->ogreAssetName.toStdString());
            assetUsed = true;
        }
        else
        {
            texture_names.push_back(textureList[i].ref.toStdString());
        }

    if ( assetUsed )
    {
     
         Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
         if (!materialPtr.isNull() && texture_names.size() == cSkyBoxTextureCount)
         {
            materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setCubicTextureName(&texture_names[0], false);
         }
    }
    
    CreateSky();
    
}

void EC_SkyBox::OnAttributeUpdated(IAttribute* attribute)
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
        while(textureAssets.size() > textures.Size())
            textureAssets.pop_back();
        while(textureAssets.size() < textures.Size())
            textureAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));

        for(int i = 0; i < textures.Size(); ++i)
        {
            connect(textureAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnTextureAssetLoaded(AssetPtr)), Qt::UniqueConnection);
            textureAssets[i]->HandleAssetRefChange(framework_->Asset(), textures[i].ref);
        }

     
        //SetTextures();
    }
}

void EC_SkyBox::SetTextures()
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
        for(int i = 0; i < texture_names.size(); ++i)
            materialPtr->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setFrameTextureName(Ogre::String(texture_names[i].c_str()), i);

    DisableSky();
    CreateSky();
}

void EC_SkyBox::DisableSky()
{
    if (!ViewEnabled())
        return;

    if (renderer_.expired())
        renderer_.lock()->GetSceneManager()->setSkyBox(false, "");
}

}
