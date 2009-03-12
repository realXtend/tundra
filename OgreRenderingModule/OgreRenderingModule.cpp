// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"
#include "EC_OgreEntity.h"

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterface_Impl(type_static_)
    {
    }

    OgreRenderingModule::~OgreRenderingModule()
    {
    }

    // virtual
    void OgreRenderingModule::Load()
    {
        using namespace OgreRenderer;

        LogInfo("Module " + Name() + " loaded.");
        DECLARE_MODULE_EC(EC_OgreEntity);
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::Initialize(Foundation::Framework *framework)
    {
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework));
        renderer_->Initialize();

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void OgreRenderingModule::Uninitialize(Foundation::Framework *framework)
    {        
        renderer_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void OgreRenderingModule::Update()
    {
        renderer_->Update();
    }
}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

