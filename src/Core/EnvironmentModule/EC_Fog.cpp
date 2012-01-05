// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define MATH_OGRE_INTEROP

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
    mode(this, "Mode", 3),
    color(this,"Color", Color(0.707792f,0.770537f,0.831373f,1.f)),
    startDistance(this, "Start distance", 100.f),
    endDistance(this, "End distance", 2000.f),
    expDensity(this, "Exponential density", 0.001f)
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

    // Only when rendering is enabled
    if (!framework->IsHeadless())
    {
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(Update()), Qt::UniqueConnection);
        connect(this, SIGNAL(ParentEntitySet()), SLOT(Update()));
    }
}

EC_Fog::~EC_Fog()
{
    if (framework->IsHeadless())
        return;
    OgreWorldPtr w = world.lock();
    if (!w)
        return;

    w->GetSceneManager()->setFog(Ogre::FOG_NONE);
    w->GetRenderer()->MainViewport()->setBackgroundColour(Color()); // Color default ctor == black
}

void EC_Fog::Update()
{
    if (framework->IsHeadless())
        return;
    if (!ParentScene())
        return;
    OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
    if (!w)
        return;

    world = w;
    // Note: in Tundra1-series, if we were within EC_WaterPlane, the waterPlaneColor*fogColor was used as the scene fog color.
    w->GetSceneManager()->setFog((Ogre::FogMode)mode.Get(), color.Get(), expDensity.Get(), startDistance.Get(), endDistance.Get());
    if ((FogMode)mode.Get() == None)
        w->GetRenderer()->MainViewport()->setBackgroundColour(Color()); // Color default ctor == black
    else
        w->GetRenderer()->MainViewport()->setBackgroundColour(color.Get());
}
