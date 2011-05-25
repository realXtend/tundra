// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Fog.h"

#include "AttributeMetadata.h"
#include "LoggingFunctions.h"

#include <QDebug>
#include <Ogre.h>
#include <OgreCommon.h>
#include <OgreColourValue.h>

#include "MemoryLeakCheck.h"

namespace Environment
{

EC_Fog::EC_Fog(SceneManager* scene) :
    IComponent(scene),
    startDistance(this, "Start distance", 100.f),
    endDistance(this, "End distance", 2000.f),
    color(this,"Color", Color(0.707792f,0.770537f,0.831373f,1.f)),
    mode(this, "Mode", 3),
    use(this, "Use over Caelum color", false)
{
    static AttributeMetadata metadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        metadata.enums[Ogre::FOG_NONE] = "NoFog";
        metadata.enums[Ogre::FOG_EXP] = "Exponentially";
        metadata.enums[Ogre::FOG_EXP2] = "ExponentiallySquare";
        metadata.enums[Ogre::FOG_LINEAR] = "Linearly";
        mode.SetMetadata(&metadata);
        metadataInitialized = true;
    }
}

Ogre::ColourValue EC_Fog::GetColorAsOgreValue() const
{
    Color col = color.Get();
    return Ogre::ColourValue(col.r, col.g, col.b, col.a);
}

}
