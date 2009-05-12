// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Ogre.h>

#include "OgreGtkWindowModule.h"
#include "OgreRenderingModule.h"

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
    OgreGtkWindowModule::OgreGtkWindowModule() : ModuleInterfaceImpl(type_static_),
        ogre_window_(NULL), ogre_widget_(NULL)
    {
    }

    OgreGtkWindowModule::~OgreGtkWindowModule()
    {
    }

    // virtual
    void OgreGtkWindowModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void OgreGtkWindowModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void OgreGtkWindowModule::PreInitialize()
    {
        bool embed = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "EmbedOgreIntoGtk", false);

        boost::shared_ptr<OgreRenderingModule> rendering_module = framework_->GetModuleManager()->GetModule<OgreRenderingModule>(Foundation::Module::MT_Renderer).lock();
        Foundation::ModuleWeakPtr gtkmmui_module = framework_->GetModuleManager()->GetModule("GtkmmUI");
        
        
        if (embed && rendering_module && gtkmmui_module.expired() == false)
        {
            ogre_window_ = new Gtk::Window();
            ogre_widget_ = new OgreWidget(rendering_module->GetRenderer());
            ogre_window_->set_border_width(10);
            ogre_window_->add(*ogre_widget_);
            ogre_widget_->show();
            ogre_window_->show();
            ogre_window_->signal_hide().connect(sigc::mem_fun(*this, &OgreGtkWindowModule::OnOgreGtkWindowClosed));

#ifdef _WINDOWS
            // Must be after ogre_window_->show(), the window is not properly created until then.
            Core::uint handle = 0;
            Glib::RefPtr<Gdk::Window> main_gdk_win = ogre_window_->get_window();
            handle = (Core::uint)(GDK_WINDOW_HWND(main_gdk_win->gobj()));
            rendering_module->GetRenderer()->SetMainWindowHandle(handle);
#endif
            //! \todo Retrieve main Gtk window handle for other platforms.
        }
    }

    // virtual
    void OgreGtkWindowModule::Initialize()
    {        
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void OgreGtkWindowModule::PostInitialize()
    {
    }

    // virtual 
    void OgreGtkWindowModule::Uninitialize()
    {        
        delete ogre_window_;
        delete ogre_widget_;
        
        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    void OgreGtkWindowModule::OnOgreGtkWindowClosed()
    {
        boost::shared_ptr<OgreRenderingModule> rendering_module = framework_->GetModuleManager()->GetModule<OgreRenderingModule>(Foundation::Module::MT_Renderer).lock();
        if (rendering_module && rendering_module->GetRenderer())
        {
            rendering_module->GetRenderer()->OnWindowClosed();
        }
    }
}


