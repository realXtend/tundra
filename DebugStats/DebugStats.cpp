#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
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
#include "SceneModule.h"
#include "ComponentInterface.h"

#include "DebugStats.h"
#include "GtkmmUI.h"

#include "EC_Viewable.h"
#include "EC_FreeData.h"
#include "EC_SpatialSound.h"
#include "EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"
#include "QuatUtils.h"

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
    
    InitializePrimPropertiesWindow();

    Log("Module " + Name() + " initialized.");
}

void DebugStats::PostInitialize()
{
    scene_event_category_ = framework_->GetEventManager()->QueryEventCategory("Scene");
    if (scene_event_category_ == 0)
        Log("Unable to find event category for Scene events!");    
}

void DebugStats::Uninitialize()
{
    Gtk::Window *debugWindow = 0;
    debugModules_->get_widget("windowDebugModules", debugWindow);
    SAFE_DELETE(debugWindow);
    SAFE_DELETE(windowEntityList);
    SAFE_DELETE(primPropertiesWindow_);
}

void DebugStats::Update(Core::f64 frametime)
{
}

bool DebugStats::HandleEvent(
    Core::event_category_id_t category_id,
    Core::event_id_t event_id,
    Foundation::EventDataInterface *data
    )
{
    if (category_id == scene_event_category_)
    {
        Scene::SceneEventData *event_data = dynamic_cast<Scene::SceneEventData *>(data);    
        switch(event_id)
        {
            case Scene::EVENT_SCENE_ADDED:
            case Scene::EVENT_SCENE_DELETED:
            case Scene::EVENT_ENTITY_ADDED:
            case Scene::EVENT_ENTITY_UPDATED:
            case Scene::EVENT_ENTITY_DELETED:
            case Scene::EVENT_ENTITY_SELECTED:
                UpdateEntityListTreeView(event_id, event_data);
                break;
            default:
                break;
        }
    }
    
    return false;
}

void DebugStats::Log(const std::string &str)
{
    Poco::Logger::get("DebugStats").information(str);
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
        Foundation::ModuleInterface *m = modules[i].module_.get();

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

void DebugStats::InitializeEntityListWindow()
{
    // Load up the debug module hierarchy window, and store the main window handle for later use.
    entityListControls_ = Gnome::Glade::Xml::create("data/entityListWindow.glade");
    if (!entityListControls_)
        return;
    
    // Set up tree view.
    Gtk::TreeView *treeview_entitylist = 0;
    entityListControls_->get_widget("treeview_entitylist", treeview_entitylist);
    
    // Set data model and column names.
    entityListModel_ = Gtk::TreeStore::create(entityModelColumns_);
    treeview_entitylist->set_model(entityListModel_);
    treeview_entitylist->append_column(Glib::ustring("Name"), entityModelColumns_.colName);
    treeview_entitylist->append_column(Glib::ustring("ID"), entityModelColumns_.colID);
    
    // Bind callback for the refresh button.
    entityListControls_->connect_clicked("button_refresh", sigc::mem_fun(*this, &DebugStats::OnClickRefresh));
    // Bind callback for the double-click on EC.
    treeview_entitylist->signal_row_activated().connect(sigc::mem_fun(*this, &DebugStats::OnDoubleClickEntity));
    
    // Show, set title, set default size.
    entityListControls_->get_widget("window_entitylist", windowEntityList);
    windowEntityList->set_default_size(250, 300);
    windowEntityList->show();
    windowEntityList->set_title("Entity List");
}

void DebugStats::PopulateEntityListTreeView()
{
    using namespace std;
    
    Gtk::TreeView *treeview_entitylist = 0;
    entityListControls_->get_widget("treeview_entitylist", treeview_entitylist);

    Scene::SceneManager *sceneManager = dynamic_cast<Scene::SceneManager *>
        (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!sceneManager)
        return;

    //Fill the TreeView's model.
    entityListModel_->clear();
    
    const Scene::SceneManager::SceneMap &scenes = sceneManager->GetSceneMap();
    for(Scene::SceneManager::SceneMap::const_iterator iter = scenes.begin(); iter != scenes.end(); ++iter)
    {
        // Add scene node.
        const Foundation::SceneInterface &scene = *iter->second;
        
        Gtk::TreeModel::Row scene_row = *(entityListModel_->append());
        scene_row[entityModelColumns_.colName] = scene.Name();
        scene_row[entityModelColumns_.colID] = "";
        
        for(Foundation::SceneInterface::ConstEntityIterator iter = scene.begin(); iter != scene.end(); ++iter)
        {
            // Add entity.
            const Scene::Entity &entity = dynamic_cast<const Scene::Entity &>(*iter);

            Gtk::TreeModel::Row entity_row = *(entityListModel_->append(scene_row.children()));
            entity_row[entityModelColumns_.colName] = "Entity";
            entity_row[entityModelColumns_.colID] = Core::ToString(entity.GetId());
            
            const Scene::Entity::ComponentVector &components = entity.GetComponentVector();
            for(Scene::Entity::ComponentVector::const_iterator iter = components.begin(); iter != components.end(); ++iter)
            {
                // Add component.
                const Foundation::ComponentInterfacePtr &component = dynamic_cast<const Foundation::ComponentInterfacePtr &>(*iter); 
                Gtk::TreeModel::Row component_row = *(entityListModel_->append(entity_row.children()));
                component_row[entityModelColumns_.colName] = component->Name();
                component_row[entityModelColumns_.colID] = "";
            }
        }
    }
}

void DebugStats::UpdateEntityListTreeView(Core::event_id_t event_id, Scene::SceneEventData *event_data)
{
    Scene::SceneManager *scene_manager = dynamic_cast<Scene::SceneManager *>
        (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!scene_manager)
            return;
            
    switch(event_id)
    {
        case Scene::EVENT_SCENE_ADDED:
            {
                ///\todo Test, the events don't make it this far for now...
                Gtk::TreeModel::Row scene_row;
                scene_row = *(entityListModel_->append());
                scene_row[entityModelColumns_.colName] = event_data->sceneName;
                scene_row[entityModelColumns_.colID] = "";
                break;
            }
        case Scene::EVENT_SCENE_DELETED:
            {
                ///\todo Make work & test.
                /*Gtk::TreeModel::Children rows = entityListModel_->children();
                Gtk::TreeModel::Children::iterator iter; 
                if (!rows)
                    return;
                                
                for(iter = rows.begin(); iter != rows.end(); ++iter)
                {
                    const Gtk::TreeModel::Row &row = *iter;
                    if (row[entityModelColumns_.colName] == event_data->sceneName)
                        break;
                }
                
                entityListModel_->erase(iter);*/
                break;
            }
        case Scene::EVENT_ENTITY_SELECTED:
            {
                ///\todo Get the real scene, not hardcoded
                const Foundation::ScenePtr &scene = scene_manager->GetScene("World");
                const Foundation::EntityPtr &entity = scene->GetEntity(event_data->localID);
                const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
                const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
                if (!prim_component || !ogre_component)
                    return;
                
                RexLogic::EC_OpenSimPrim *prim = dynamic_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
                OgreRenderer::EC_OgrePlaceable *ogre_pos = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());        
                PopulatePrimPropertiesTreeView(prim, ogre_pos);                
                break;
            }        
        case Scene::EVENT_ENTITY_ADDED:            
            {
                // Find the scene where this entity belongs to. ///\todo Implemented better?
                std::string scene_name;
                const Scene::SceneManager::SceneMap &scenes = scene_manager->GetSceneMap();
                for(Scene::SceneManager::SceneMap::const_iterator iter = scenes.begin(); iter != scenes.end(); ++iter)
                {
                    const Foundation::SceneInterface &scene = *iter->second;
                    if(scene.HasEntity(event_data->localID))
                        scene_name = scene.Name();
                }
                
                // Get the scene for real.
                const Foundation::ScenePtr &scene_ptr = scene_manager->GetScene(scene_name);
                if (!scene_ptr)
                    return;
                const Foundation::SceneInterface &scene = *scene_ptr.get();

                // Find the scene row or create it if it doesn't exist.                
                Gtk::TreeModel::Children rows = entityListModel_->children();
                Gtk::TreeModel::Children::iterator iter;
                Gtk::TreeModel::Row scene_row;
                bool found = false;
                
                for(iter = rows.begin(); iter != rows.end(); ++iter)
                {
                    scene_row = *iter;
                    if (scene_row[entityModelColumns_.colName] == scene.Name())
                    {
                        found = true;
                        break;
                    }
                }
                
                ///\todo Remove when adding scenes works properly.
                if (!found)
                {
                    scene_row = *(entityListModel_->append());
                    scene_row[entityModelColumns_.colName] = scene.Name();
                    scene_row[entityModelColumns_.colID] = "";
                }
                    
                // Add entity.
                const Foundation::EntityPtr &entity = scene.GetEntity(event_data->localID);
                const Scene::Entity &ent = dynamic_cast<const Scene::Entity &>(*entity.get());

                Gtk::TreeModel::Row entity_row = *(entityListModel_->append(scene_row.children()));
                entity_row[entityModelColumns_.colName] = "Entity";
                entity_row[entityModelColumns_.colID] = Core::ToString(ent.GetId());

                const Scene::Entity::ComponentVector &components = ent.GetComponentVector();
                for(Scene::Entity::ComponentVector::const_iterator iter = components.begin(); iter != components.end(); ++iter)
                {
                    // Add component. 
                    const Foundation::ComponentInterfacePtr &component = dynamic_cast<const Foundation::ComponentInterfacePtr &>(*iter); 
                    Gtk::TreeModel::Row component_row = *(entityListModel_->append(entity_row.children()));
                    component_row[entityModelColumns_.colName] = component->Name();
                    component_row[entityModelColumns_.colID] = "";
                }
            }
            break;
        case Scene::EVENT_ENTITY_UPDATED:
        case Scene::EVENT_ENTITY_DELETED:
        case Scene::EVENT_COMPONENT_ADDED:
        case Scene::EVENT_COMPONENT_DELETED:
        default:
            break;
    }
}

void DebugStats::OnClickRefresh()
{
    PopulateEntityListTreeView();
}

void DebugStats::OnClickSave()
{
    Gtk::SpinButton *sb_pos_x, *sb_pos_y, *sb_pos_z, *sb_scale_x, *sb_scale_y,
        *sb_scale_z, *sb_rot_x, *sb_rot_y, *sb_rot_z;
    
    primPropertiesControls_->get_widget("sb_pos_x", sb_pos_x);
    primPropertiesControls_->get_widget("sb_pos_y", sb_pos_y);
    primPropertiesControls_->get_widget("sb_pos_z", sb_pos_z);
    primPropertiesControls_->get_widget("sb_scale_x", sb_scale_x);
    primPropertiesControls_->get_widget("sb_scale_y", sb_scale_y);
    primPropertiesControls_->get_widget("sb_scale_z", sb_scale_z);
    primPropertiesControls_->get_widget("sb_rot_x", sb_rot_x);
    primPropertiesControls_->get_widget("sb_rot_y", sb_rot_y);
    primPropertiesControls_->get_widget("sb_rot_z", sb_rot_z);
    
    ///\todo Get the real scene, not hardcoded
    Scene::SceneManager *scene_manager = dynamic_cast<Scene::SceneManager *>
        (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!scene_manager)
            return;    
    const Foundation::ScenePtr &scene = scene_manager->GetScene("World");
    const Foundation::EntityPtr &entity = scene->GetEntity(currentEntityID_);
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
    OgreRenderer::EC_OgrePlaceable *ogre_pos = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());        
    
    // Get the new values.
    RexTypes::Vector3 pos((float)sb_pos_x->get_value(), (float)sb_pos_y->get_value(), (float)sb_pos_z->get_value());
    RexTypes::Vector3 scale((float)sb_scale_x->get_value(), (float)sb_scale_y->get_value(), (float)sb_scale_z->get_value());
    Quaternion quat = Core::UnpackQuaternionFromFloat3((float)sb_rot_x->get_value(), (float)sb_rot_y->get_value(), (float)sb_rot_z->get_value());
    
    // Set the new values.
    ogre_pos->SetPosition(pos);
    ogre_pos->SetScale(scale);
    ogre_pos->SetOrientation(quat);
    
    //Send event
    Scene::SceneEventData event_data(currentEntityID_);
    event_data.entity_ptr_list.push_back(entity);
    framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::EVENT_ENTITY_UPDATED, &event_data);    
}

void DebugStats::OnClickCancel()
{

}

void DebugStats::OnPrimPropertiesClose()
{
    // Send 'Entity Deselect' event.
    Scene::SceneEventData event_data(currentEntityID_);
    framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::EVENT_ENTITY_DESELECT, &event_data);    
    currentEntityID_ = 0;
}

void DebugStats::OnDoubleClickEntity(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn* column)
{
    Scene::SceneManager *scene_manager = dynamic_cast<Scene::SceneManager *>
        (framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager));
    if (!scene_manager)
        return;

    Gtk::TreeModel::iterator iter = entityListModel_->get_iter(path);
    if (!iter)
        return;
        
    Gtk::TreeModel::Row row = *iter;
    
    if(row[entityModelColumns_.colName] == "Entity")
    {
        Core::entity_id_t id;
        try
        {
            id = Core::ParseString<Core::entity_id_t>(row[entityModelColumns_.colID]);
        } catch(boost::bad_lexical_cast)
        {
            return;
        }
        
        currentEntityID_ = id;
        // Send 'Entity Selected' event.
        Scene::SceneEventData event_data(id);
        framework_->GetEventManager()->SendEvent(scene_event_category_, Scene::EVENT_ENTITY_SELECT, &event_data);

        ///\todo Get the real scene, not hardcoded
        const Foundation::ScenePtr &scene = scene_manager->GetScene("World");
        const Foundation::EntityPtr &entity = scene->GetEntity(id);
        const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
        const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
        if (!prim_component || !ogre_component)
            return;
        
        RexLogic::EC_OpenSimPrim *prim = dynamic_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
        OgreRenderer::EC_OgrePlaceable *ogre_pos = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());        
        
        PopulatePrimPropertiesTreeView(prim, ogre_pos);
   }
}

void DebugStats::InitializePrimPropertiesWindow()
{
    // Load up the debug module hierarchy window, and store the main window handle for later use.
    primPropertiesControls_ = Gnome::Glade::Xml::create("data/primPropertiesWindow.glade");
    if (!primPropertiesControls_)
        return;
    
    // Get the window.
    primPropertiesControls_->get_widget("dialog_prim_properties", primPropertiesWindow_);
    
    // Set up tree view.
    Gtk::TreeView *treeview_prim_properties = 0;
    primPropertiesControls_->get_widget("treeview_prim_properties", treeview_prim_properties);
    
    // Set data model and column names.
    primPropertiesModel_ = Gtk::TreeStore::create(primPropertiesColumns_);
    treeview_prim_properties->set_model(primPropertiesModel_);
    treeview_prim_properties->append_column(Glib::ustring("Name"), primPropertiesColumns_.colName);
    treeview_prim_properties->append_column(Glib::ustring("Editable"), primPropertiesColumns_.colEditable);
    treeview_prim_properties->append_column_editable(Glib::ustring("Value"), primPropertiesColumns_.colValue);
    
    // Callbacks for Save and Cancel buttons
    primPropertiesControls_->connect_clicked("button_save", sigc::mem_fun(*this, &DebugStats::OnClickSave));
    primPropertiesControls_->connect_clicked("button_cancel", sigc::mem_fun(*this, &DebugStats::OnClickCancel));
    
    // Set the window title, set default size and callback.
    primPropertiesWindow_->set_default_size(400, 380);
    primPropertiesWindow_->set_title("Entity Properties");
    primPropertiesWindow_->signal_hide().connect(sigc::mem_fun(*this, &DebugStats::OnPrimPropertiesClose));
 }

void DebugStats::PopulatePrimPropertiesTreeView(
    RexLogic::EC_OpenSimPrim *prim,
    OgreRenderer::EC_OgrePlaceable *ogre_pos)
{
    primPropertiesWindow_->show();
    primPropertiesModel_->clear();
    
    // Ogre position, scale and orientation
    Gtk::SpinButton *sb_pos_x, *sb_pos_y, *sb_pos_z, *sb_scale_x,
        *sb_scale_y, *sb_scale_z, *sb_rot_x, *sb_rot_y, *sb_rot_z;
    
    primPropertiesControls_->get_widget("sb_pos_x", sb_pos_x);
    primPropertiesControls_->get_widget("sb_pos_y", sb_pos_y);
    primPropertiesControls_->get_widget("sb_pos_z", sb_pos_z);
    primPropertiesControls_->get_widget("sb_scale_x", sb_scale_x);
    primPropertiesControls_->get_widget("sb_scale_y", sb_scale_y);
    primPropertiesControls_->get_widget("sb_scale_z", sb_scale_z);
    primPropertiesControls_->get_widget("sb_rot_x", sb_rot_x);
    primPropertiesControls_->get_widget("sb_rot_y", sb_rot_y);
    primPropertiesControls_->get_widget("sb_rot_z", sb_rot_z);
    
    RexTypes::Vector3 pos = ogre_pos->GetPosition();
    RexTypes::Vector3 scale = ogre_pos->GetScale();
    RexTypes::Vector3 rot = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
    
    // Set the values
    sb_pos_x->set_value((double)pos.x);
    sb_pos_y->set_value((double)pos.y);
    sb_pos_z->set_value((double)pos.z);

    sb_scale_x->set_value((double)scale.x);
    sb_scale_y->set_value((double)scale.y);
    sb_scale_z->set_value((double)scale.z);
    
    sb_rot_x->set_value((double)rot.x);
    sb_rot_y->set_value((double)rot.y);
    sb_rot_z->set_value((double)rot.z);
    
    // Prim stuff
    Gtk::TreeModel::Row prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "Name";
    prim_row[primPropertiesColumns_.colValue] = prim->ObjectName;    
    prim_row[primPropertiesColumns_.colEditable] = true;
    
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "Description";
    prim_row[primPropertiesColumns_.colValue] = prim->Description;
    prim_row[primPropertiesColumns_.colEditable] = true;
        
    prim_row = *(primPropertiesModel_->append());        
    prim_row[primPropertiesColumns_.colName] = "RegionHandle";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString(prim->RegionHandle);
    prim_row[primPropertiesColumns_.colEditable] = false;
        
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "LocalId";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString(prim->LocalId);
    prim_row[primPropertiesColumns_.colEditable] = false;
    
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "FullID";
    prim_row[primPropertiesColumns_.colValue] = prim->FullId.ToString();
    prim_row[primPropertiesColumns_.colEditable] = false;
    
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "ParentId";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString(prim->ParentId);
    prim_row[primPropertiesColumns_.colEditable] = false;
        
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "Material";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString((Core::uint)prim->Material);
    prim_row[primPropertiesColumns_.colEditable] = true;
    
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "ClickAction";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString((Core::uint)prim->ClickAction);
    prim_row[primPropertiesColumns_.colEditable] = true;
    
    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "UpdateFlags";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString(prim->UpdateFlags);
    prim_row[primPropertiesColumns_.colEditable] = false;

    prim_row = *(primPropertiesModel_->append());
    prim_row[primPropertiesColumns_.colName] = "SelectPriority";
    prim_row[primPropertiesColumns_.colValue] = Core::ToString(prim->SelectPriority);
    prim_row[primPropertiesColumns_.colEditable] = true;
}
