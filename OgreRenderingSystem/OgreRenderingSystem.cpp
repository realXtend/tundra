// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingSystem.h"
#include "EC_OgreEntity.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"

namespace OgreRenderer
{
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

        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void OgreRenderingSystem::uninitialize(Foundation::Framework *framework)
    {        
        mRenderer.reset();

        LOG("System " + name() + " uninitialized.");
    }

}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingSystem)
POCO_END_MANIFEST

