// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_SkyBox.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
//DEFINE_POCO_LOGGING_FUNCTIONS("EC_SkyBox")

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
     connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));

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
            // This code block is not currently working, but if for some reason GetTextureNamesFromMaterialn understands cubic_textures this codeblock is runned
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
     lastTextures_ = textureRefs.Get();
}

EC_SkyBox::~EC_SkyBox()
{
    DisableSky();
    renderer_.reset();
}

void EC_SkyBox::CreateSky()
{
    if (!ViewEnabled())
        return;

    if (renderer_.expired())
        return;

    QString currentMaterial = materialRef.Get().ref;

    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
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

void EC_SkyBox::View(const QString &attributeName)
{
    /// @todo implement this.
}

void EC_SkyBox::AttributeUpdated(IAttribute* attribute)
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
        // What texture has changed?
        SetTextures();
    }
}

void EC_SkyBox::SetTextures()
{
    if (!ViewEnabled())
        return;

    AssetReferenceList lst = textureRefs.Get();
    std::vector<std::string> texture_names;
    texture_names.reserve(6);

    for(int i = 0; i < lst.Size() && i <= 6; ++i)
        texture_names.push_back(lst[i].ref.toStdString());

    Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(materialRef.Get().ref.toStdString().c_str());
    if (!materialPtr.isNull() && texture_names.size() == 6)
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
