// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include <Ogre.h>
#include "OgreRenderingModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
#include "EC_OgrePlaceable.h"

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterface_Impl(type_static_), framework_(NULL)
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
        DECLARE_MODULE_EC(EC_OgrePlaceable);
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::Initialize(Foundation::Framework *framework)
    {
        framework_  = framework;
        
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework));
        renderer_->Initialize();
        
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void OgreRenderingModule::PostInitialize(Foundation::Framework *framework)
    {
        Console::CommandService *console = framework->GetService<Console::CommandService>
            (Foundation::Service::ST_ConsoleCommand);

        console->RegisterCommand("SetViewportColor", "Set viewport background color. Usage: SetViewportColor(r,g,b)", Console::Bind(this, &OgreRenderingModule::ConsoleSetViewportColor));
    }

    // virtual 
    void OgreRenderingModule::Uninitialize(Foundation::Framework *framework)
    {        
        framework->GetServiceManager()->UnregisterService(renderer_.get());
    
        renderer_.reset();

        framework_ = NULL;
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void OgreRenderingModule::Update()
    {
        renderer_->Update();
    }

    Console::CommandResult OgreRenderingModule::ConsoleSetViewportColor(const Core::StringVector &params)
    {
        
        if (params.size() != 3)
        {
            return Console::ResultFailure("Usage: SetViewportColor(r,g,b)");
        }

        Ogre::ColourValue color;
        try
        {
            color = Ogre::ColourValue(Core::ParseString<Core::Real>(params[0]), Core::ParseString<Core::Real>(params[1]), Core::ParseString<Core::Real>(params[2]));
        } catch (std::exception)
        {
            return Console::ResultInvalidParameters();
        }

        Ogre::Camera *camera = renderer_->GetCurrentCamera();
        if (camera == NULL)
            return Console::ResultFailure("No camera or viewport");

        camera->getViewport()->setBackgroundColour(color);
        return Console::ResultSuccess();
    }
}

using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

