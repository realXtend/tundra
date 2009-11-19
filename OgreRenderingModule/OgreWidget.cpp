// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

/*

#include "Renderer.h"

#include <Ogre.h>

#pragma warning( push )
#pragma warning( disable : 4250 )
#include "OgreWidget.h"
#include <gdkmm/drawable.h>
#include <gdkmm/general.h>  // for cairo helper function

#ifdef WIN32
#include <gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif

#pragma warning( pop )

namespace OgreRenderer
{

    OgreWidget::OgreWidget(RendererPtr renderer) :
        Glib::ObjectBase("ogrewidget"),
        Gtk::Widget(),
        renderer_(renderer)
    {
      set_flags(Gtk::NO_WINDOW);
    }

    OgreWidget::~OgreWidget()
    {
    }

    void OgreWidget::on_size_request(Gtk::Requisition* requisition)
    {
        *requisition = Gtk::Requisition();

        Foundation::Framework* framework = renderer_->GetFramework();

        int width = framework->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_width", 800);
        int height = framework->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_height", 600);

        requisition->width = width;
        requisition->height = height;
    }

    void OgreWidget::on_size_allocate(Gtk::Allocation& allocation)
    {
        set_allocation(allocation);

        if (refgdkwindow_)
        {
            refgdkwindow_->move_resize( allocation.get_x(), allocation.get_y(),
                allocation.get_width(), allocation.get_height() );
        }
      
        renderer_->Resize(allocation.get_width(), allocation.get_height());   
        on_expose_event(0);
    }

    void OgreWidget::on_map()
    {
        Gtk::Widget::on_map();
    }

    void OgreWidget::on_unmap()
    {
        Gtk::Widget::on_unmap();
    }

    void OgreWidget::on_realize()
    {
        Gtk::Widget::on_realize();
       
        Gtk::Allocation allocation = get_allocation();
             
        if(!refgdkwindow_)
        {
            GdkWindowAttr attributes;
            memset(&attributes, 0, sizeof(attributes));

            attributes.x = allocation.get_x();
            attributes.y = allocation.get_y();
            attributes.width = allocation.get_width();
            attributes.height = allocation.get_height();

            attributes.event_mask = get_events () | Gdk::EXPOSURE_MASK | Gdk::ALL_EVENTS_MASK ;
            attributes.window_type = GDK_WINDOW_CHILD;
            attributes.wclass = GDK_INPUT_OUTPUT;

            refgdkwindow_ = Gdk::Window::create(get_window(), &attributes, GDK_WA_X | GDK_WA_Y);
        }
       
        if (!renderer_->IsInitialized())
        {
            std::string external_param;
    #ifdef WIN32
            external_param = Ogre::StringConverter::toString((unsigned long)GDK_WINDOW_HWND(refgdkwindow_->gobj()));
    #else
            GdkWindow* parent = refgdkwindow_->gobj();
            GdkDisplay* display = gdk_drawable_get_display(GDK_DRAWABLE(parent));
            GdkScreen* screen = gdk_drawable_get_screen(GDK_DRAWABLE(parent));

            Display* xdisplay = GDK_DISPLAY_XDISPLAY(display);
            Screen* xscreen = GDK_SCREEN_XSCREEN(screen);
            int screen_number = XScreenNumberOfScreen(xscreen);
            XID xid_parent = GDK_WINDOW_XWINDOW(parent);

            external_param = Ogre::StringConverter::toString(reinterpret_cast<unsigned long>(xdisplay)) + ":" +
            Ogre::StringConverter::toString(static_cast<unsigned int>(screen_number)) + ":" +
            Ogre::StringConverter::toString(static_cast<unsigned long>(xid_parent)); 
    #endif

            renderer_->SetExternalWindowParameter(external_param);
          
            unset_flags(Gtk::NO_WINDOW);
            set_window(refgdkwindow_);

            set_double_buffered(false);

            refgdkwindow_->set_user_data(gobj());
            refgdkwindow_->set_back_pixmap(Glib::RefPtr<Gdk::Pixmap>(), false);
        }
    }

    void OgreWidget::on_unrealize()
    {
        refgdkwindow_.clear();

        Gtk::Widget::on_unrealize();
    }

    bool OgreWidget::on_expose_event(GdkEventExpose* event)
    {
        renderer_->Render();

        return true;
    }

}

*/
