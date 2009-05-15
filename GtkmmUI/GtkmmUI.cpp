#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include "StableHeaders.h"

#include "GtkmmUI.h"

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(GtkmmUI)
POCO_END_MANIFEST

/// PImpl internals, to hide clients of GtkmmUI of the gtkmm details.
class GtkmmUIImpl
{
public:
    GtkmmUIImpl() {}

    // The Gtk main handle - we'll be doing manual iterations of the main loop instead of letting this control the app loop.
    boost::shared_ptr<Gtk::Main> gtk;
};

GtkmmUI::GtkmmUI()
:ModuleInterfaceImpl("GtkmmUI"),
main_window_(NULL)
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

void GtkmmUI::Initialize()
{
    assert(impl_);
}

void GtkmmUI::Uninitialize()
{
}

void GtkmmUI::Update(Core::f64 frametime)
{
    assert(impl_);

    impl_->gtk->iteration(false);
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Framework *framework);
void SetProfiler(Foundation::Framework *framework)
{
    Foundation::ProfilerSection::SetProfiler(&framework->GetProfiler());
}
