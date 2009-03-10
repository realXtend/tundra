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
    void OgreRenderingModule::Load()
    {
        using namespace OgreRenderer;

        LOG("Module " + Name() + " loaded.");
        DECLARE_MODULE_EC(EC_OgreEntity);
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LOG("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::Initialize(Foundation::Framework *framework)
    {
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer);

        LOG("Module " + Name() + " initialized.");
    }

    // virtual 
    void OgreRenderingModule::Uninitialize(Foundation::Framework *framework)
    {        
        renderer_.reset();

        LOG("Module " + Name() + " uninitialized.");
    }

}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

