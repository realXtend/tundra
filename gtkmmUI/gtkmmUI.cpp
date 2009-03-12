#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <libglademm.h>
#include <glade/glade.h>

#include "CoreDefines.h"

#include "gtkmmUI.h"

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(GtkmmUI)
POCO_END_MANIFEST

class GtkmmUIImpl
{
public:
    GtkmmUIImpl():debugWindow(0) {}

    // The Gtk main handle - we'll be doing manual iterations of the main loop instead of letting this control the app loop.
    boost::shared_ptr<Gtk::Main> gtk;
    // Handle to a treeview of a listing of currently running modules, for debugging purposes.
    Glib::RefPtr<Gnome::Glade::Xml> debugModules;
    // The window of the treeview control that shows running modules.
    Gtk::Window *debugWindow;
};

GtkmmUI::GtkmmUI()
:ModuleInterface_Impl("GtkmmUI")
{
    impl_ = boost::shared_ptr<GtkmmUIImpl>(new GtkmmUIImpl);
}

GtkmmUI::~GtkmmUI()
{
}

void GtkmmUI::Load()
{
    impl_->gtk = boost::shared_ptr<Gtk::Main>(new Gtk::Main(0, 0));
}

void GtkmmUI::Unload()
{
}

void GtkmmUI::Initialize(Foundation::Framework *framework)
{
    assert(impl_);

    // Load up the debug module hierarchy window, and store the main window handle for later use.

    impl_->debugModules = Gnome::Glade::Xml::create("data/DbgModules.glade");
    if (!impl_->debugModules)
        return;

    impl_->debugModules->get_widget("windowDebugModules", impl_->debugWindow);

    if (!impl_->debugWindow)
        return;

    impl_->debugWindow->show();
}

void GtkmmUI::Uninitialize(Foundation::Framework *framework)
{
    SAFE_DELETE(impl_->debugWindow);
}

void GtkmmUI::Update()
{
    assert(impl_);

    impl_->gtk->iteration(false);
}
