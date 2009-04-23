// For conditions of distribution and use, see copyright notice in license.txt

/// Please note: The code of this module can be pretty fugly from time to time.
/// This module is just meant for quick testing and debugging stuff.

#ifndef incl_DebugStats_h
#define incl_DebugStats_h

#include "ModuleInterface.h"
#include "EventDataInterface.h"

namespace RexLogic
{
    class EC_OpenSimPrim;
}
namespace OgreRenderer
{
    class EC_OgrePlaceable;
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

    bool HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id,
        Foundation::EventDataInterface *data
        );

private:
    void operator=(const DebugStats &);
    DebugStats(const DebugStats &);

    static void Log(const std::string &str); 
    
    /// Callback for Entity List refresh button.
    void OnClickRefresh();
    
    /// Callback for closing Entity Properties window.
    void OnPrimPropertiesClose();
    
    /// Callback for Save button of Entity Properties window.
    void OnClickSave();
    
    /// Callback for Cancel button of Entity Properties window.
    void OnClickCancel();
    
    /// Update entity list.
    void UpdateEntityListTreeView(Core::event_id_t event_id, Scene::Events::SceneEventData *event_data);
    
    /// Callback for Entity activation.
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

    /// Show EC_OpenSimPrim and EC_OgrePlaceable properties.    
    void PopulatePrimPropertiesTreeView(
        RexLogic::EC_OpenSimPrim *prim,
        OgreRenderer::EC_OgrePlaceable *ogre_pos
        );
    
    /// Category id for scene events.
    Core::event_category_id_t scene_event_category_ ;
    
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
        PrimPropertiesModelColumns()
        {
            add(colName);
            add(colValue);
            add(colEditable);
        }
        Gtk::TreeModelColumn<Glib::ustring> colName;
        Gtk::TreeModelColumn<Glib::ustring> colValue;
        Gtk::TreeModelColumn<bool> colEditable;
    };
            
    const PrimPropertiesModelColumns primPropertiesColumns_;
    
    /// Currenty selected entity ID on the PrimProperties window.
    Core::entity_id_t currentEntityID_;
    
    //For the validated column:
//    Gtk::CellRendererText cellrendererValidated;
//    Gtk::TreeView::Column columnValidated;
//    bool validateRetry;
//    Glib::ustring invalidTextForRetry;        
};

#endif
