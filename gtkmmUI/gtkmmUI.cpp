#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <libglademm.h>
#include <glade/glade.h>

#include "StableHeaders.h"

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

    struct ModelColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring>  moduleName;

        ModelColumns() { add(moduleName); }
    };

    const ModelColumns moduleModelColumns;
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

    Gtk::TreeView *tv = 0;
    impl_->debugModules->get_widget("treeview1", tv);

    Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(impl_->moduleModelColumns);
    Gtk::TreeRow row = *model->append();
    row[impl_->moduleModelColumns.moduleName] = Glib::ustring("jee");
    Gtk::TreeStore::iterator iter = model->append();
    iter->set_value(0, Glib::ustring("jee"));
    Gtk::TreeStore::iterator iter2 = model->insert(iter);
    iter2->set_value(0, Glib::ustring("lapsi"));
    iter = model->append();
    iter->set_value(0, Glib::ustring("jee2"));
    tv->set_model(model);

    tv->append_column(Glib::ustring("Name"), impl_->moduleModelColumns.moduleName);
    Gtk::CellRendererText *r = dynamic_cast<Gtk::CellRendererText*>(tv->get_column_cell_renderer(0));
    r->property_xalign().set_value(0.0);

    assert(r);
    r->property_foreground_gdk().set_value(Gdk::Color(Glib::ustring("808080")));
    r->property_font_desc().set_value(Pango::FontDescription(Glib::ustring("Arial 12")));
    

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
