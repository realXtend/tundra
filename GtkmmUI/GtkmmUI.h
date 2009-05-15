#ifndef incl_gtkmmUI_h
#define incl_gtkmmUI_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "GtkmmUIApi.h"

class GtkmmUIImpl;

namespace Gtk
{
    class Window;
}

/// GtkmmUI is a module that implements and provides Gtkmm UI for other modules.
class GTKMMUI_MODULE_API GtkmmUI : public Foundation::ModuleInterfaceImpl
{
public:
    GtkmmUI();
    virtual ~GtkmmUI();

    void Load();
    void Unload();
    void Initialize();
    void Uninitialize();
    void Update(Core::f64 frametime);

    void SetMainWindow(Gtk::Window* window) { main_window_ = window; }
    Gtk::Window* GetMainWindow() { return main_window_; }

private:
    void operator=(const GtkmmUI &);
    GtkmmUI(const GtkmmUI &);

    Foundation::Framework *framework_;

    boost::shared_ptr<GtkmmUIImpl> impl_;

    Gtk::Window* main_window_;
};

#endif
