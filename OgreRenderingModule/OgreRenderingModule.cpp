// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "EC_OgreEntity.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterface_Impl(Foundation::Module::Type_Renderer)
    {
    }

    OgreRenderingModule::~OgreRenderingModule()
    {
    }

    // virtual
    void OgreRenderingModule::load()
    {
        using namespace OgreRenderer;

        LOG("System " + name() + " loaded.");
        DECLARE_MODULE_EC(EC_OgreEntity);
    }

    // virtual
    void OgreRenderingModule::unload()
    {
        LOG("System " + name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::initialize(Foundation::Framework *framework)
    {
        mRenderer = OgreRenderer::RendererPtr(new OgreRenderer::Renderer);

        LOG("System " + name() + " initialized.");
    }

    // virtual 
    void OgreRenderingModule::uninitialize(Foundation::Framework *framework)
    {        
        mRenderer.reset();

        LOG("System " + name() + " uninitialized.");
    }

}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

