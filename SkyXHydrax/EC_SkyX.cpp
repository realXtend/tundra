/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.cpp
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#include "DebugOperatorNew.h"

#include "EC_SkyX.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "LoggingFunctions.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <Ogre.h>

#include <SkyX.h>

#include "MemoryLeakCheck.h"

struct EC_SkyXImpl
{
    EC_SkyXImpl() : skyX(0) {}
    ~EC_SkyXImpl()
    {
        skyX->remove();
        SAFE_DELETE(skyX)
    }

    SkyX::SkyX *skyX;
};

EC_SkyX::EC_SkyX(Scene* scene) :
    IComponent(scene)
{
    try
    {
        OgreWorldPtr w = scene->GetWorld<OgreWorld>();
        // Create Sky
        impl = new EC_SkyXImpl();
        impl->skyX = new SkyX::SkyX(w->GetSceneManager(),
            static_cast<EC_Camera *>(w->GetRenderer()->GetActiveCamera())->GetCamera());
        impl->skyX->create();

        // Add a basic cloud layer
        impl->skyX->getCloudsManager()->add(SkyX::CloudLayer::Options()); //Default options 
        // Volumetric clouds
    //    impl->skyX->getVCloudsManager()->create();

        // A little change to default atmosphere settings :)
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.RayleighMultiplier = 0.0045f;
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);

        connect(framework_->Frame(), SIGNAL(Updated(float)), SLOT(Update(float)));
    }
    catch(Ogre::Exception &e)
    {
        LogError("Could not create EC_SkyX: " + std::string(e.what()));
    }
}

EC_SkyX::~EC_SkyX()
{
    delete impl;
}

void EC_SkyX::Update(float frameTime)
{
    impl->skyX->update(frameTime);
}
