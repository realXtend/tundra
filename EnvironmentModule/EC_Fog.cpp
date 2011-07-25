// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define OGRE_INTEROP

#include "EC_Fog.h"

#include "Scene.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "OgreWorld.h"
#include "Renderer.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_Fog::EC_Fog(Scene* scene) :
    IComponent(scene),
    startDistance(this, "Start distance", 100.f),
    endDistance(this, "End distance", 2000.f),
    color(this,"Color", Color(0.707792f,0.770537f,0.831373f,1.f)),
    mode(this, "Mode", 3)
{
    static AttributeMetadata metadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        metadata.enums[Ogre::FOG_NONE] = "NoFog";
        metadata.enums[Ogre::FOG_EXP] = "Exponentially";
        metadata.enums[Ogre::FOG_EXP2] = "ExponentiallySquare";
        metadata.enums[Ogre::FOG_LINEAR] = "Linearly";
        mode.SetMetadata(&metadata);
        metadataInitialized = true;
    }

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(Update()), Qt::UniqueConnection);

    Update();
}

void EC_Fog::Update()
{
    if (!ParentScene())
        return;
    OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
    if (!w)
        return;

    // Note: in Tundra1-series, if we were within EC_WaterPlane, the waterPlaneColor*fogColor was used as the scene fog color.
    w->GetSceneManager()->setFog(static_cast<Ogre::FogMode>(mode.Get()), color.Get(), 0.001f, startDistance.Get(), endDistance.Get());
    w->GetRenderer()->GetViewport()->setBackgroundColour(color.Get());
}
