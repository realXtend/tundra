// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingSystem.h"
#include "EC_OgreEntity.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"

OgreRenderingSystem::OgreRenderingSystem() : ModuleInterface_Impl(Foundation::Module::Type_Renderer)
{
}

OgreRenderingSystem::~OgreRenderingSystem()
{
}

// virtual
void OgreRenderingSystem::load()
{
    using namespace OgreRenderer;

    LOG("System " + name() + " loaded.");
    DECLARE_MODULE_EC(EC_OgreEntity);
}

// virtual
void OgreRenderingSystem::unload()
{
    LOG("System " + name() + " unloaded.");
}

// virtual
void OgreRenderingSystem::initialize(Foundation::Framework *framework)
{
    mRenderer = OgreRenderer::RendererPtr(new OgreRenderer::Renderer);
    framework->getServiceManager()->registerService(Foundation::Service_Renderer, mRenderer.get());

    LOG("System " + name() + " initialized.");
}

// virtual 
void OgreRenderingSystem::uninitialize(Foundation::Framework *framework)
{
    framework->getServiceManager()->unregisterService(mRenderer.get());
    
    mRenderer.reset();

    LOG("System " + name() + " uninitialized.");
}



POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingSystem)
POCO_END_MANIFEST

