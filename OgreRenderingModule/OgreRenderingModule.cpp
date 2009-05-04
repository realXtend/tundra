// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include <Ogre.h>
#include "OgreRenderingModule.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreLight.h"
#include "EC_OgreSky.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreConsoleOverlay.h"

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/window.h>
#include "OgreWidget.h"
#pragma warning( pop )

#ifdef WIN32
#include <gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif

namespace OgreRenderer
{
    OgreRenderingModule::OgreRenderingModule() : ModuleInterfaceImpl(type_static_),
        assetcategory_id_(0),
        resourcecategory_id_(0),
        ogre_widget_(NULL),
        ogre_window_(NULL)
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
        DECLARE_MODULE_EC(EC_OgreMesh);
        DECLARE_MODULE_EC(EC_OgreLight);
        DECLARE_MODULE_EC(EC_OgreSky);
        DECLARE_MODULE_EC(EC_OgreCustomObject);
        DECLARE_MODULE_EC(EC_OgreConsoleOverlay);
    }

    // virtual
    void OgreRenderingModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreRenderingModule::PreInitialize()
    {
        // create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
        renderer_ = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_));
    }

    // virtual
    void OgreRenderingModule::Initialize()
    {      
        assert (renderer_);
        
        bool embed = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "EmbedOgreIntoGtk", false);
        
        if (!embed)
        {
            renderer_->Initialize();
        }
        else
        {
            ogre_window_ = new Gtk::Window();
            ogre_widget_ = new OgreWidget(renderer_);
            ogre_window_->set_border_width(10);
            ogre_window_->add(*ogre_widget_);
            ogre_widget_->show();
            ogre_window_->show();
            ogre_window_->signal_hide().connect(sigc::mem_fun(*this, &OgreRenderingModule::OnOgreGtkWindowClosed));

#ifdef _WINDOWS
            // Must be after ogre_window_->show(), the window is not properly created until then.
            Core::uint handle = 0;
            Glib::RefPtr<Gdk::Window> main_gdk_win = ogre_window_->get_window();
            handle = (Core::uint)(GDK_WINDOW_HWND(main_gdk_win->gobj()));
            renderer_->SetMainWindowHandle(handle);
#endif
            //! \todo Retrieve main Gtk window handle for other platforms.
 
            assert (renderer_->IsInitialized());
        }

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Renderer, renderer_.get());

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void OgreRenderingModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        assetcategory_id_ = event_manager->QueryEventCategory("Asset");
        
        if (assetcategory_id_ == 0 )
            LogWarning("Unable to find event category for Asset events!");

        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
        if (!resourcecategory_id_)
        {
            resourcecategory_id_ = event_manager->RegisterEventCategory("Resource");
            event_manager->RegisterEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, "ResourceReady");
        }

        renderer_->PostInitialize();

        // Hackish way to register renderer debug console commands
        if (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_ConsoleCommand))
        {
            Console::CommandService *console = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand);
            console->RegisterCommand(Console::CreateCommand(
                "RequestTexture", "Download, decode & create Ogre texture. Usage: RequestTexture(uuid)", 
                Console::Bind(this, &OgreRenderingModule::ConsoleRequestTexture)));
            console->RegisterCommand(Console::CreateCommand(
                "RequestMesh", "Download & create Ogre mesh. Usage: RequestMesh(uuid)", 
                Console::Bind(this, &OgreRenderingModule::ConsoleRequestMesh)));
        }
    }

    // virtual
    bool OgreRenderingModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        if (!renderer_)
            return false;

        if (category_id == assetcategory_id_)
        {
            return renderer_->HandleAssetEvent(event_id, data);
        }

        if (category_id == resourcecategory_id_)
        {
            return renderer_->HandleResourceEvent(event_id, data);
        }

        return false;
    }

    // virtual 
    void OgreRenderingModule::Uninitialize()
    {        
        if (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_ConsoleCommand))
        {
            // Unregister debug commands
            Console::CommandService *console = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand);
            console->UnregisterCommand("RequestTexture");
            console->UnregisterCommand("RequestMesh");
        }

        framework_->GetServiceManager()->UnregisterService(renderer_.get());
    
        delete ogre_window_;
        delete ogre_widget_;
        renderer_.reset();
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void OgreRenderingModule::Update(Core::f64 frametime)
    {
        renderer_->Update(frametime);
    }

    Console::CommandResult OgreRenderingModule::ConsoleRequestTexture(const Core::StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("Usage: RequestTexture(uuid)");

        if (renderer_)
            renderer_->RequestTexture(params[0]);

        return Console::ResultSuccess();
    }

    Console::CommandResult OgreRenderingModule::ConsoleRequestMesh(const Core::StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("Usage: RequestMesh(uuid)");

        if (renderer_)
            renderer_->RequestMesh(params[0]);

        return Console::ResultSuccess();
    }

    void OgreRenderingModule::OnOgreGtkWindowClosed()
    {
        if (renderer_)
            renderer_->OnWindowClosed();
    }
}


using namespace OgreRenderer;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OgreRenderingModule)
POCO_END_MANIFEST

