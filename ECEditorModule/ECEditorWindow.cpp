// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "ComponentManager.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"
#include "XmlUtilities.h"

#include <QDomDocument>
#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>

using namespace RexTypes;

namespace ECEditor
{
    uint AddUniqueItem(QListWidget* list, const QString& name)
    {
        for (int i = 0; i < list->count(); ++i)
        {
            if (list->item(i)->text() == name)
                return i;
        }
        
        list->addItem(name);
        return list->count() - 1;
    }
    
    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        framework_(framework),
        contents_(0),
        save_button_(0),
        revert_button_(0),
        create_button_(0),
        delete_button_(0),
        entity_list_(0),
        component_list_(0),
        create_combo_(0),
        data_edit_(0),
        delete_shortcut_(0)
    {
        Initialize();
    }
    
    ECEditorWindow::~ECEditorWindow()
    {
    }
    
    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(contents_);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);
        
        save_button_ = findChild<QPushButton*>("but_save");
        revert_button_ = findChild<QPushButton*>("but_revert");
        create_button_ = findChild<QPushButton*>("but_create");
        delete_button_ = findChild<QPushButton*>("but_delete");
        entity_list_ = findChild<QListWidget*>("list_entities");
        component_list_ = findChild<QListWidget*>("list_components");
        data_edit_ = findChild<QTextEdit*>("text_attredit");
        create_combo_ = findChild<QComboBox*>("combo_create");
        
        if (entity_list_)
        {
            entity_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            delete_shortcut_ = new QShortcut(QKeySequence(Qt::Key_Delete), entity_list_);
            QObject::connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshEntityComponents()));
            QObject::connect(delete_shortcut_, SIGNAL(activated()), this, SLOT(DeleteEntitiesFromList()));
        }
        if (component_list_)
        {
            component_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            QObject::connect(component_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshComponentData()));
        }
        if (delete_button_)
            QObject::connect(delete_button_, SIGNAL(pressed()), this, SLOT(DeleteComponent()));
        if (create_button_)
            QObject::connect(create_button_, SIGNAL(pressed()), this, SLOT(CreateComponent()));
        if (save_button_)
            QObject::connect(save_button_, SIGNAL(pressed()), this, SLOT(SaveData()));
        if (revert_button_)
            QObject::connect(revert_button_, SIGNAL(pressed()), this, SLOT(RevertData()));
            
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
            ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(contents_->windowTitle(), UiServices::ModuleWidget));
        else
            ECEditorModule::LogError("Could not add widget to scene");
        
        RefreshAvailableComponents();
    }
    
    void ECEditorWindow::RefreshAvailableComponents()
    {
        // Fill the create component combo box with the types of EC's the ComponentManager can create
        Foundation::ComponentManagerPtr comp_mgr = framework_->GetComponentManager();
        const Foundation::ComponentManager::ComponentFactoryMap& factories = comp_mgr->GetComponentFactoryMap();
        if (create_combo_)
        {
            create_combo_->clear();
            Foundation::ComponentManager::ComponentFactoryMap::const_iterator i = factories.begin();
            while (i != factories.end())
            {
                create_combo_->addItem(QString::fromStdString(i->first));
                ++i;
            }
        }
    }
    
    void ECEditorWindow::DeleteComponent()
    {
        if (!component_list_)
            return;
        
        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        StringVector components;
        for (uint i = 0; i < component_list_->count(); ++i)
        {
            QListWidgetItem* item = component_list_->item(i);
            if (item->isSelected())
                components.push_back(item->text().toStdString());
        }
        
        for (uint i = 0; i < entities.size(); ++i)
        {
            for (uint j = 0; j < components.size(); ++j)
                entities[i]->RemoveComponent(entities[i]->GetComponent(components[j]));
        }
        
        RefreshEntityComponents();
    }
    
    void ECEditorWindow::CreateComponent()
    {
        if (!create_combo_)
            return;
        
        std::string name = create_combo_->currentText().toStdString();
        
        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        for (uint i = 0; i < entities.size(); ++i)
        {
            // We (mis)use the GetOrCreateComponent function to avoid adding the same EC multiple times, since identifying multiple EC's of similar type
            // is problematic with current API
            entities[i]->GetOrCreateComponent(name);
        }
        
        RefreshEntityComponents();
    }
    
    void ECEditorWindow::RevertData()
    {
        RefreshComponentData();
    }
    
    void ECEditorWindow::SaveData()
    {
        if (!data_edit_)
            return;
        
        QDomDocument edited_doc;
        
        QString text = data_edit_->toPlainText();
        if (!text.length())
        {
            ECEditorModule::LogWarning("Empty XML data");
            return;
        }
        
        if (edited_doc.setContent(text))
        {
            Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
            bool entity_found = false;
            
            // Check if multi-entity or single-entity
            QDomElement entities_elem = edited_doc.firstChildElement("entities");
            
            // Deserialize all entities/components contained in the data, provided we still find them from the scene
            QDomElement entity_elem;
            if (!entities_elem.isNull())
                entity_elem = entities_elem.firstChildElement("entity");
            else
                entity_elem = edited_doc.firstChildElement("entity");
            
            while (!entity_elem.isNull())
            {
                entity_found = true;
                entity_id_t id = (entity_id_t)ParseInt(entity_elem.attribute("id").toStdString());
                Scene::EntityPtr entity = scene->GetEntity(id);
                if (entity)
                {
                    QDomElement comp_elem = entity_elem.firstChildElement("component");
                    while (!comp_elem.isNull())
                    {
                        Foundation::ComponentInterfacePtr comp = entity->GetComponent(comp_elem.attribute("type").toStdString());
                        if (comp)
                            comp->DeserializeFrom(comp_elem);
                        comp_elem = comp_elem.nextSiblingElement("component");
                    }
                }
                else
                {
                    ECEditorModule::LogWarning("Could not find entity " + ToString<int>(id) + " in scene!");
                }
                
                entity_elem = entity_elem.nextSiblingElement("entity");
            }
            
            // Refresh immediately after, so that any extra stuff is stripped out, and illegal parameters are (hopefully) straightened
            if (entity_found)
                RefreshComponentData();
            else
                ECEditorModule::LogWarning("No entity elements in XML data");
        }
        else
            ECEditorModule::LogWarning("Could not parse XML data");
    }
    
    void ECEditorWindow::hideEvent(QHideEvent* hide_event)
    {
        ClearEntities();
        QWidget::hideEvent(hide_event);
    }
    
    void ECEditorWindow::showEvent(QShowEvent* show_event)
    {
        RefreshAvailableComponents();
        QWidget::showEvent(show_event);
    }
    
    void ECEditorWindow::DeleteEntitiesFromList()
    {
        if ((entity_list_) && (entity_list_->hasFocus()))
        {
            for (int i = entity_list_->count() - 1; i >= 0; --i)
            {
                if (entity_list_->item(i)->isSelected())
                {
                    QListWidgetItem* item = entity_list_->takeItem(i);
                    delete item;
                }
            }
        }
    }
    
    void ECEditorWindow::AddEntity(entity_id_t entity_id)
    {
        if ((isVisible()) && (entity_list_))
        {
            QString entity_id_str;
            entity_id_str.setNum((int)entity_id);
            
            entity_list_->setCurrentRow(AddUniqueItem(entity_list_, entity_id_str));
        }
    }
    
    void ECEditorWindow::RefreshEntityComponents()
    {
        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        
        // If no entities selected, just clear the list and we're done
        if (!entities.size())
        {
            component_list_->clear();
            return;
        }
        
        std::vector<QString> added_components;
        
        for (uint i = 0; i < entities.size(); ++i)
        {
            const Scene::Entity::ComponentVector& components = entities[i]->GetComponentVector();

            for (uint j = 0; j < components.size(); ++j)
            {
                QString component_name = QString::fromStdString(components[j]->TypeName());
                added_components.push_back(component_name);
                // If multiple selected entities have the same component, add it only once to the EC selector list
                AddUniqueItem(component_list_, QString::fromStdString(components[j]->TypeName()));
            }
        }
        
        // Now delete components that should not be in the component list
        // This code is kind of ugly...
        // Note: the whole reason for going to this trouble (instead of just nuking and refilling the list)
        // is to retain the user's selection, if several entities share a set of components, as often is the case
        for (int i = component_list_->count() - 1; i >= 0; --i)
        {
            bool found = false;
            QListWidgetItem* item = component_list_->item(i);
            for (uint j = 0; j < added_components.size(); ++j)
            {
                if (item->text() == added_components[j])
                {
                    found = true;
                    break;
                }
            }
            
            if (!found)
            {
                QListWidgetItem* item = component_list_->takeItem(i);
                delete item;
            }
        }
        
        RefreshComponentData();
    }
    
    void ECEditorWindow::RefreshComponentData()
    {
        if (!data_edit_)
            return;
        
        data_edit_->clear();
        
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size())
            return;
            
        QDomDocument temp_doc;
        if (selection.size() > 1)
        {
            // Multi-entity: create a root element to hold entities
            QDomElement entities_elem = temp_doc.createElement("entities");
            temp_doc.appendChild(entities_elem);
            
            for (uint i = 0; i < selection.size(); ++i)
            {
                QDomElement entity_elem = temp_doc.createElement("entity");
                QString id_str;
                id_str.setNum(selection[i].entity_->GetId());
                entity_elem.setAttribute("id", id_str);
                for (uint j = 0; j < selection[i].components_.size(); ++j)
                    selection[i].components_[j]->SerializeTo(temp_doc, entity_elem);
                entities_elem.appendChild(entity_elem);
            }
        }
        else
        {
            // Single entity
            QDomElement entity_elem = temp_doc.createElement("entity");
            temp_doc.appendChild(entity_elem);
            
            QString id_str;
            id_str.setNum(selection[0].entity_->GetId());
            entity_elem.setAttribute("id", id_str);
            for (uint j = 0; j < selection[0].components_.size(); ++j)
                selection[0].components_[j]->SerializeTo(temp_doc, entity_elem);
            temp_doc.appendChild(entity_elem);
        }
        
        data_edit_->setText(temp_doc.toString());
    }
    
    std::vector<Scene::EntityPtr> ECEditorWindow::GetSelectedEntities()
    {
        std::vector<Scene::EntityPtr> ret;
        
        if (!entity_list_)
            return ret;
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        
        for (uint i = 0; i < entity_list_->count(); ++i)
        {
            QListWidgetItem* item = entity_list_->item(i);
            if (item->isSelected())
            {
                entity_id_t id = (entity_id_t)item->text().toInt();
                Scene::EntityPtr entity = scene->GetEntity(id);
                if (entity)
                    ret.push_back(entity);
            }
        }
        
        return ret;
    }
    
    std::vector<EntityComponentSelection> ECEditorWindow::GetSelectedComponents()
    {
        std::vector<EntityComponentSelection> ret;
        
        if (!component_list_)
            return ret;
        
        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        StringVector components;
        
        for (uint i = 0; i < component_list_->count(); ++i)
        {
            QListWidgetItem* item = component_list_->item(i);
            if (item->isSelected())
                components.push_back(item->text().toStdString());
        }
        
        for (uint i = 0; i < entities.size(); ++i)
        {
            EntityComponentSelection selection;
            selection.entity_ = entities[i];
            for (uint j = 0; j < components.size(); ++j)
            {
                Foundation::ComponentInterfacePtr comp = selection.entity_->GetComponent(components[j]);
                if (comp)
                    selection.components_.push_back(comp);
            }
            
            ret.push_back(selection);
        }
        
        return ret;
    }
    
    void ECEditorWindow::ClearEntities()
    {
        if (entity_list_)
            entity_list_->clear();
        if (component_list_)
            component_list_->clear();
    }
}

