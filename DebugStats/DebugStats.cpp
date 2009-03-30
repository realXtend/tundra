#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include <sstream>

#include <Poco/Logger.h>

#include "StableHeaders.h"
#include "Foundation.h"

#include "SceneManager.h"
#include "Entity.h"
#include "SceneInterface.h"
#include "ComponentInterface.h"

#include "DebugStats.h"
#include "GtkmmUI.h"

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(DebugStats)
POCO_END_MANIFEST

DebugStats::DebugStats()
:ModuleInterfaceImpl("DebugStats")
{
}

DebugStats::~DebugStats()
{
}

void DebugStats::Load()
{
}

void DebugStats::Unload()
{
}

void DebugStats::Initialize()
{
    InitializeModulesWindow();
    PopulateModulesTreeView();
    
    InitializeEventsWindow();
    PopulateEventsTreeView();
    
    InitializeEntityListWindow();
}

void DebugStats::PostInitialize()
{
    /*eventCategoryID_ = framework_->GetEventManager()->QueryEventCategory("Scene");
        
    if (eventCategoryID_  == 0)
        Log("Unable to find event category for Scene events!");*/
}

void DebugStats::Uninitialize()
{
    Gtk::Window *debugWindow = 0;
    debugModules_->get_widget("windowDebugModules", debugWindow);
    SAFE_DELETE(debugWindow);
    SAFE_DELETE(windowEntityList);
}

void DebugStats::Update(Core::f64 frametime)
{
}

bool DebugStats::HandleEvent(
    Core::event_category_id_t category_id,
    Core::event_id_t event_id, 
    Foundation::EventDataInterface* data)
{
    return false;
}

void DebugStats::Log(const std::string &str)
{
    Poco::Logger::get("DebugStats").information(str);
}

void DebugStats::InitializeEntityListWindow()
{
    // Load up the debug module hierarchy window, and store the main window handle for later use.
    entityListControls_ = Gnome::Glade::Xml::create("data/entityListWindow.glade");
    if (!entityListControls_)
        return;
    
    // Get the window.
    entityListControls_->get_widget("window_entitylist", windowEntityList);
    
    // Show scroll bars only when necessary
    Gtk::ScrolledWindow *scrolledwindow_entitylist = 0;
    entityListControls_->get_widget("scrolledwindow_entitylist", scrolledwindow_entitylist);
    scrolledwindow_entitylist->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // Set up tree view.
    Gtk::TreeView *treeview_entitylist = 0;
    entityListControls_->get_widget("treeview_entitylist", treeview_entitylist);
    
    // Set data model and column names.
    entityListModel_ = Gtk::TreeStore::create(entityModelColumns_);
    treeview_entitylist->set_model(entityListModel_);
    treeview_entitylist->append_column(Glib::ustring("ID"), entityModelColumns_.colID);
    treeview_entitylist->append_column(Glib::ustring("Name"), entityModelColumns_.colName);
    
    // Bind callback for the refresh button.
    //Gtk::Button *button_refresh = 0;
    //entityListControls_->get_widget("button_refresh", button_refresh);
    entityListControls_->connect_clicked("button_refresh", sigc::mem_fun(*this, &DebugStats::OnClickRefresh));
    /*button_refresh->signal_clicked().connect(sigc::mem_fun(*this, &DebugStats::OnClickRefresh));
    button_refresh->set_border_width(5);
    button_refresh->set_layout(Gtk::BUTTONBOX_END);*/
    
    // Show, set title, set default size.
    windowEntityList->set_default_size(150, 200);
    windowEntityList->show();
    windowEntityList->set_title("Entity List");
}

void DebugStats::PopulateEntityListTreeView()
{
    using namespace std;
    
    Gtk::TreeView *treeview_entitylist = 0;
    entityListControls_->get_widget("treeview_entitylist", treeview_entitylist);

    entityListModel_->clear();

    Scene::SceneManager *sceneManager = dynamic_cast<Scene::SceneManager *>
        (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!sceneManager)
        return;

    //Fill the TreeView's model.
    const Scene::SceneManager::SceneMap &scenes = sceneManager->GetSceneMap();
    for(Scene::SceneManager::SceneMap::const_iterator iter = scenes.begin(); iter != scenes.end(); ++iter)
    {
        // Add scene node.
        const Foundation::SceneInterface &scene = *iter->second;
        
        Gtk::TreeModel::Row scene_row = *(entityListModel_->append());
        scene_row[entityModelColumns_.colID] = "";
        scene_row[entityModelColumns_.colName] = scene.Name();
        
        for(Foundation::SceneInterface::ConstEntityIterator iter = scene.begin(); iter != scene.end(); ++iter)
        {
            // Add entity.
            const Scene::Entity &entity = dynamic_cast<const Scene::Entity &>(*iter);

            Gtk::TreeModel::Row entity_row = *(entityListModel_->append(scene_row.children()));
            entity_row[entityModelColumns_.colID] = Core::ToString(entity.GetId());
            entity_row[entityModelColumns_.colName] = "Entity";
            
            const Scene::Entity::ComponentVector &components = entity.GetComponentVector();
            for(Scene::Entity::ComponentVector::const_iterator iter = components.begin(); iter != components.end(); ++iter)
            {
                // Add component. 
                const Foundation::ComponentInterfacePtr &component = dynamic_cast<const Foundation::ComponentInterfacePtr &>(*iter); 
                Gtk::TreeModel::Row component_row = *(entityListModel_->append(entity_row.children()));
                component_row[entityModelColumns_.colID] = "";
                component_row[entityModelColumns_.colName] = component->Name();
            }
        }
    }
}

void DebugStats::InitializeModulesWindow()
{
    // Load up the debug module hierarchy window, and store the main window handle for later use.
    debugModules_ = Gnome::Glade::Xml::create("data/DbgModules.glade");
    if (!debugModules_)
        return;

    Gtk::Window *debugWindow = 0;
    debugModules_->get_widget("windowDebugModules", debugWindow);

    debugModulesModel_ = Gtk::TreeStore::create(moduleModelColumns_);

    Gtk::TreeView *tv = 0;
    debugModules_->get_widget("treeview1", tv);
    tv->set_model(debugModulesModel_);

    debugWindow->show();
}

void DebugStats::PopulateModulesTreeView()
{
    Gtk::Window *debugWindow = 0;
    debugModules_->get_widget("windowDebugModules", debugWindow);
    
    Gtk::TreeView *tv = 0;
    debugModules_->get_widget("treeview1", tv);
    tv->append_column(Glib::ustring("Name"), moduleModelColumns_.moduleName);

    debugModulesModel_->clear();

    Foundation::ModuleManager &mm = *framework_->GetModuleManager();
    const Foundation::ModuleManager::ModuleVector &modules = mm.GetModuleList();

    for(size_t i = 0; i < modules.size(); ++i)
    {
        Foundation::ModuleInterface *m = modules[i].module_;

        Gtk::TreeStore::iterator iter = debugModulesModel_->append();
        if (!m)
        {
            iter->set_value(0, Glib::ustring("Null"));
        }
        else
        {
            iter->set_value(0, m->Name()); ///\todo Type to string.
        }
    }
}

void DebugStats::InitializeEventsWindow()
{
    // Load up the debug module hierarchy window, and store the main window handle for later use.
    debugEvents_ = Gnome::Glade::Xml::create("data/DbgEvents.glade");
    if (!debugEvents_)
        return;

    Gtk::Window *debugWindow = 0;
    debugEvents_->get_widget("window1", debugWindow);

    debugEventsModel_ = Gtk::TreeStore::create(moduleModelColumns_);

    Gtk::TreeView *tv = 0;
    debugEvents_->get_widget("treeview1", tv);
    tv->set_model(debugEventsModel_);

    debugWindow->show();
}

void DebugStats::PopulateEventsTreeView()
{
    Gtk::Window *debugWindow = 0;
    debugEvents_->get_widget("window1", debugWindow);
    
    Gtk::TreeView *tv = 0;
    debugEvents_->get_widget("treeview1", tv);
    tv->append_column(Glib::ustring("Name"), moduleModelColumns_.moduleName);

    debugEventsModel_->clear();

    Foundation::EventManager &em = *framework_->GetEventManager();
    const Foundation::EventManager::EventCategoryMap &categories = em.GetEventCategoryMap();

    for(Foundation::EventManager::EventCategoryMap::const_iterator iter = categories.begin();
        iter != categories.end(); ++iter)
    {
        std::stringstream ss;
        ss << iter->first << " (id:" << iter->second << ")";

        Gtk::TreeStore::iterator treeiter = debugEventsModel_->append();

        treeiter->set_value(0, ss.str());
    }
}

void DebugStats::OnClickRefresh()
{
    PopulateEntityListTreeView();
}