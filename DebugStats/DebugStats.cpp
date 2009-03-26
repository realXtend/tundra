#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include <sstream>

#include <Poco/Logger.h>

#include "StableHeaders.h"
#include "Foundation.h"

#include "../SceneModule/SceneManager.h"
#include "../SceneModule/Entity.h"
#include "../Interfaces/SceneInterface.h"
#include "DebugStats.h"
#include "GtkmmUI.h"

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(DebugStats)
POCO_END_MANIFEST

DebugStats::DebugStats()
:ModuleInterface_Impl("DebugStats")
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

void DebugStats::PostInitialize()
{
    InitializeModulesWindow();
    PopulateModulesTreeView();

    InitializeEventsWindow();
    PopulateEventsTreeView();
}

void DebugStats::InitializeObjectsWindow()
{

}

void DebugStats::Log(const std::string &str)
{
    Poco::Logger::get("DebugStats").information(str);
}

void DebugStats::PopulateObjectsTreeView()
{
    using namespace std;

    Scene::SceneManager *sceneManager = dynamic_cast<Scene::SceneManager *>(framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!sceneManager)
        return;

    const Scene::SceneManager::SceneMap &scenes = sceneManager->GetSceneMap();
    for(Scene::SceneManager::SceneMap::const_iterator iter = scenes.begin(); iter != scenes.end(); ++iter)
    {
        // Add scene node.
        const Foundation::SceneInterface &scene = *iter->second;
        Log(string("Scene: ") +  scene.Name());

        for(Foundation::SceneInterface::ConstEntityIterator iter = scene.begin(); iter != scene.end(); ++iter)
        {
            const Scene::Entity &entity = dynamic_cast<const Scene::Entity &>(*iter); // from Foundation::EntityInterface &
            // Add entity.
            stringstream ss;
            ss << " Entity: " << entity.GetId();
            Log(ss.str());
            const Scene::Entity::ComponentVector components = entity.GetComponentVector();
            for(Scene::Entity::ComponentVector::const_iterator iter = components.begin(); iter != components.end(); ++iter)
            {
                // Add component.
                //const Foundation::ComponentInterface &component = **iter;
                //Log(string("  Component: ") + component._Name());

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

void DebugStats::Uninitialize()
{
    Gtk::Window *debugWindow = 0;
    debugModules_->get_widget("windowDebugModules", debugWindow);
    SAFE_DELETE(debugWindow);
}

void DebugStats::Update()
{
}
