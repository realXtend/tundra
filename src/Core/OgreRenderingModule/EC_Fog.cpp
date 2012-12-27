// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define MATH_OGRE_INTEROP

#include "EC_Fog.h"

#include "Scene/Scene.h"
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
        metadataInitialized = true;
    }
    mode.SetMetadata(&metadata);

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
    if (w)
        w->SetDefaultSceneFog();
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

    // Specify the fog color.
    if (w->OgreSceneManager())
        w->OgreSceneManager()->setFog((Ogre::FogMode)mode.Get(), color.Get(), expDensity.Get(), startDistance.Get(), endDistance.Get());

    // Specify the window background color to match the fog color.
    Ogre::Viewport *ovp = w->Renderer() ? w->Renderer()->MainViewport() : 0;
    if (ovp)
    {
        if ((FogMode)mode.Get() == None)
            ovp->setBackgroundColour(Color()); // Color default ctor == black
        else
            ovp->setBackgroundColour(color.Get());
    }
}
