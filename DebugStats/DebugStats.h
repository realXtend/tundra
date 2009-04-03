#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "ModuleInterface.h"

namespace RexLogic
{
    class EC_OpenSimPrim;
}

/// This module shows information about internal core data structures in separate windows. Useful for verifying and understanding
/// the internal state of the application.
class DebugStats : public Foundation::ModuleInterfaceImpl
{
public:
    DebugStats();
    virtual ~DebugStats();

    void Load();
    void Unload();
    void Initialize();
    void PostInitialize();
    void Uninitialize();

    void Update(Core::f64 frametime);

    bool HandleEvent(Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data);

private:
    void operator=(const DebugStats &);
    DebugStats(const DebugStats &);

    static void Log(const std::string &str); 
    
    /// Callback for Entity List refresh button.
    void OnClickRefresh();
    
    void OnDoubleClickEntity(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn* column);
    
    /// Initialize UI windows.
    void InitializeModulesWindow();
    void InitializeEventsWindow();
    void InitializeEntityListWindow();
    void InitializePrimPropertiesWindow();
    
    /// Fill TreeViews with data.
    void PopulateModulesTreeView();
    void PopulateEventsTreeView();
    void PopulateEntityListTreeView();
    void PopulatePrimPropertiesTreeView(RexLogic::EC_OpenSimPrim *prim);
    
    /// Category id for scene events.
    Core::event_category_id_t eventCategoryID_ ;
    
    /// Data related to the window that shows currently loaded modules, for debugging purposes.
    Glib::RefPtr<Gnome::Glade::Xml> debugModules_;
    Glib::RefPtr<Gtk::TreeStore> debugModulesModel_;

    struct ModelColumns : public Gtk::TreeModelColumnRecord
    {
        Gtk::TreeModelColumn<Glib::ustring>  moduleName;

        ModelColumns() { add(moduleName); }
    };

    const ModelColumns moduleModelColumns_;
   
    // Data related to the window that shows the registered event categories.
    Glib::RefPtr<Gnome::Glade::Xml> debugEvents_;
    Glib::RefPtr<Gtk::TreeStore> debugEventsModel_;
    
    // Data related to the window that shows the entity list of the scene.
    Glib::RefPtr<Gnome::Glade::Xml> entityListControls_;
    Glib::RefPtr<Gtk::TreeStore> entityListModel_;
    
    Gtk::Window *windowEntityList;
    
    /// Tree model columns for entity list.
    class EntityModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        EntityModelColumns() { add(colID); add(colName); }
        Gtk::TreeModelColumn<Glib::ustring> colName;
        Gtk::TreeModelColumn<std::string> colID;
    };
            
    const EntityModelColumns entityModelColumns_;

    // Data related to the window that shows the entity list of the scene.
    Glib::RefPtr<Gnome::Glade::Xml> primPropertiesControls_;
    Glib::RefPtr<Gtk::TreeStore> primPropertiesModel_;
    
    Gtk::Window *primPropertiesWindow_;
    
    /// Tree model columns for EC_OpenSimPrim properties.

    class PrimPropertiesModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        PrimPropertiesModelColumns() { add(colName); add(colValue); }
        Gtk::TreeModelColumn<Glib::ustring> colName;
        Gtk::TreeModelColumn<Glib::ustring> colValue;
    };
            
    const PrimPropertiesModelColumns primPropertiesColumns_;    
};

#endif
