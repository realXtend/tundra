#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "ModuleInterface.h"

/// This module shows information about internal core data structures in separate windows. Useful for verifying and understanding
/// the internal state of the application.
class DebugStats : public Foundation::ModuleInterface_Impl
{
public:
    DebugStats();
    virtual ~DebugStats();

    void Load();
    void Unload();
    void Initialize(Foundation::Framework *framework) {}
    void PostInitialize(Foundation::Framework *framework);
    void Uninitialize(Foundation::Framework *framework);

    void Update();

private:
    void operator=(const DebugStats &);
    DebugStats(const DebugStats &);

    Foundation::Framework *framework_;

    // Data related to the window that shows currently loaded modules, for debugging purposes.
    
    Glib::RefPtr<Gnome::Glade::Xml> debugModules_;
    Glib::RefPtr<Gtk::TreeStore> debugModulesModel_;

    struct ModelColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring>  moduleName;

        ModelColumns() { add(moduleName); }
    };

    const ModelColumns moduleModelColumns_;

    void InitializeModulesWindow();
    void PopulateModulesTreeView();

    void InitializeEventsWindow();
    void PopulateEventsTreeView();

    // Data related to the window that shows the registered event categories.

    Glib::RefPtr<Gnome::Glade::Xml> debugEvents_;
    Glib::RefPtr<Gtk::TreeStore> debugEventsModel_;
};

#endif
