// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ECLightEditor.h"

#include "ModuleManager.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "ComponentManager.h"
#include "XMLUtilities.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "CoreStringUtils.h"

#include <qttreepropertybrowser.h>
#include <qtpropertymanager.h>
#include <qteditorfactory.h>
#include <qtvariantproperty.h>

#include <QtProperty>

#include <QApplication>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QGraphicsProxyWidget>
#include <QTreeWidget>
#include <QHeaderView>

#include "MemoryLeakCheck.h"

using namespace RexTypes;

namespace ECEditor
{
    uint AddUniqueListItem(QListWidget* list, const QString& name)
    {
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->text() == name)
                return i;

        list->addItem(name);
        return list->count() - 1;
    }

    uint AddTreeItem(QTreeWidget *list, const QString &name, int entity_id)
    {
        for(int i = 0; i < list->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *existing = list->topLevelItem(i);
            if (existing && existing->text(0) == name)
            {
                // We have already item for this EC. Create a dummy parent for the existing EC item and
                // the new one we're adding if it's not already.
                ///\todo Check/test if the code block below is required for real.
                if (existing->text(1) == "(Multiple)")
                {
                    // It's already dummy parent. Add new item to its child.
                    QTreeWidgetItem *item = new QTreeWidgetItem(existing);
                    item->setText(0, name);
                    item->setText(1, QString::number(entity_id));
                    existing->addChild(item);
                    return i;
                }

                // The existing item is not dummy parent yet, make it now.
                QTreeWidgetItem *dummyParent = new QTreeWidgetItem(list);
                dummyParent->setText(0, name);
                dummyParent->setText(1, "(Multiple)");

                // Relocate the existing item from the top level to a child of the dummy parent.
                existing = list->takeTopLevelItem(i);
                dummyParent->addChild(existing);
                list->addTopLevelItem(dummyParent);

                // Finally, create new item for this EC.
                QTreeWidgetItem *item = new QTreeWidgetItem(dummyParent);
                item->setText(0, name);
                item->setText(1, QString::number(entity_id));
                dummyParent->addChild(item);
                return i;
            }
        }

        // No existing top level item, create one now.
        QTreeWidgetItem *item = new QTreeWidgetItem(list);
        item->setText(0, name);
        item->setText(1, QString::number(entity_id));
        list->addTopLevelItem(item);
        return list->topLevelItemCount() - 1;
    }

    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        QWidget(),
        framework_(framework),
        contents_(0),
        save_button_(0),
        revert_button_(0),
        create_button_(0),
        delete_button_(0),
        toggle_browser_button_(0),
        entity_list_(0),
        component_list_(0),
        create_combo_(0),
        data_edit_(0),
        property_browser_(0),
        delete_shortcut_(0)
    {
        Initialize();
    }

    ECEditorWindow::~ECEditorWindow()
    {
        while(!editorEntityList_.empty())
        {
            SAFE_DELETE(editorEntityList_.begin()->second)
            editorEntityList_.erase(editorEntityList_.begin());
        }

        // Explicitily delete component list because it's parent of dynamically allocated items.
        SAFE_DELETE(component_list_);
    }
    
    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
        file.close();

        //Intialize property browser, manager and factory for future use.
        property_browser_ = new QtTreePropertyBrowser();
        QVBoxLayout *property_layout = findChild<QVBoxLayout *>("verticalLayout_properties");
        property_layout->addWidget(property_browser_);
        property_browser_->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        property_browser_->hide();
        
        variantManager_ = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        property_browser_->setFactoryForManager(variantManager_, variantFactory);
        QObject::connect(variantManager_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(contents_);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);
        setWindowTitle(contents_->windowTitle());

        save_button_ = findChild<QPushButton*>("but_save");
        revert_button_ = findChild<QPushButton*>("but_revert");
        create_button_ = findChild<QPushButton*>("but_create");
        delete_button_ = findChild<QPushButton*>("but_delete");
        toggle_browser_button_ = findChild<QPushButton *>("but_show_properties");
        entity_list_ = findChild<QListWidget*>("list_entities");
        component_list_ = findChild<QTreeWidget*>("list_components");
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
            component_list_->header()->setResizeMode(QHeaderView::ResizeToContents);
            component_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            QObject::connect(component_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshComponentData()));
            QObject::connect(component_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshPropertyBrowser()));
        }
        if (delete_button_)
            QObject::connect(delete_button_, SIGNAL(pressed()), this, SLOT(DeleteComponent()));
        if (create_button_)
            QObject::connect(create_button_, SIGNAL(pressed()), this, SLOT(CreateComponent()));
        if (save_button_)
            QObject::connect(save_button_, SIGNAL(pressed()), this, SLOT(SaveData()));
        if (revert_button_)
            QObject::connect(revert_button_, SIGNAL(pressed()), this, SLOT(RevertData()));
        if (toggle_browser_button_)
            QObject::connect(toggle_browser_button_, SIGNAL(pressed()), this, SLOT(TogglePropertiesBrowser()));

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
        //for (uint i = 0; i < component_list_->count(); ++i)
        for(uint i = 0; i < component_list_->topLevelItemCount(); ++i)
        {
            //QListWidgetItem* item = component_list_->item(i);
            QTreeWidgetItem *item = component_list_->topLevelItem(i);
            if (item->isSelected())
                components.push_back(item->text(0).toStdString());
        }

        for (uint i = 0; i < entities.size(); ++i)
            for (uint j = 0; j < components.size(); ++j)
                entities[i]->RemoveComponent(entities[i]->GetComponent(components[j]), Foundation::Local);

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
            entities[i]->GetOrCreateComponent(name, Foundation::Local);
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
            if (!scene)
                return;
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
                        {
                            comp->DeserializeFrom(comp_elem, Foundation::Local);
                            // Trigger sync to network
                            comp->ComponentChanged(Foundation::Local);
                        }
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
    
    void ECEditorWindow::changeEvent(QEvent *e)
    {
        if (e->type() == QEvent::LanguageChange)
        {
            QString title = TR("ECEditor", "Entity-component Editor");
            graphicsProxyWidget()->setWindowTitle(title);
        }
        else
           QWidget::changeEvent(e);
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
            
            entity_list_->setCurrentRow(AddUniqueListItem(entity_list_, entity_id_str));
        }
    }

    void ECEditorWindow::RefreshEntityComponents()
    {
        for(int i = component_list_->topLevelItemCount() - 1; i >= 0; --i)
        {
            QTreeWidgetItem *item = component_list_->takeTopLevelItem(i);
            SAFE_DELETE(item);
        }

        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        // If no entities selected, just clear the list and we're done
        if (!entities.size())
            return;
/*
        {
            //component_list_->clear();
            return;
        }
*/
        std::vector<QString> added_components;
        for (uint i = 0; i < entities.size(); ++i)
        {
            const Scene::Entity::ComponentVector& components = entities[i]->GetComponentVector();
            for (uint j = 0; j < components.size(); ++j)
            {
                QString component_name = components[j]->TypeName().c_str();
                added_components.push_back(component_name);
                AddTreeItem(component_list_, components[j]->TypeName().c_str(), entities[i]->GetId());
            }
        }

        // Now delete components that should not be in the component list
        // Note: the whole reason for going to this trouble (instead of just nuking and refilling the list)
        // is to retain the user's selection, if several entities share a set of components, as often is the case
        //for (int i = component_list_->count() - 1; i >= 0; --i)
/*
        for (uint i = 0; i < component_list_->topLevelItemCount(); ++i)
        {
            bool found = false;
            //QListWidgetItem* item = component_list_->item(i);
            QTreeWidgetItem* item = component_list_->topLevelItem(i);
            for (uint j = 0; j < added_components.size(); ++j)
            {
                //if (item->text() == added_components[j])
                if (item->text(0) == added_components[j])
                {
                    found = true;
                    break;
                }
            }
            
            if (!found)
            {
                //QListWidgetItem* item = component_list_->takeItem(i);
                QTreeWidgetItem* item = component_list_->takeTopLevelItem(i);
                for(uint k = 0; k < item->childCount(); ++k)
                {
                    QTreeWidgetItem *child = (item->takeChild(k));
                    SAFE_DELETE(child);
                }
                SAFE_DELETE(item);
            }
        }
*/
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
                id_str.setNum((int)selection[i].entity_->GetId());
                entity_elem.setAttribute("id", id_str);
                for (uint j = 0; j < selection[i].components_.size(); ++j)
                {
                    if (selection[i].components_[j]->IsSerializable())
                        selection[i].components_[j]->SerializeTo(temp_doc, entity_elem);
                }
                entities_elem.appendChild(entity_elem);
            }
        }
        else
        {
            // Single entity
            QDomElement entity_elem = temp_doc.createElement("entity");
            temp_doc.appendChild(entity_elem);
            
            QString id_str;
            id_str.setNum((int)selection[0].entity_->GetId());
            entity_elem.setAttribute("id", id_str);
            for (uint j = 0; j < selection[0].components_.size(); ++j)
            {
                if (selection[0].components_[j]->IsSerializable())
                    selection[0].components_[j]->SerializeTo(temp_doc, entity_elem);
            }
            temp_doc.appendChild(entity_elem);
        }
        
        data_edit_->setText(temp_doc.toString());
    }

    void ECEditorWindow::RefreshPropertyBrowser()
    {
        if (!property_browser_)
            return;
        
        property_browser_->clear();
        while(!editorEntityList_.empty())
        {
            SAFE_DELETE(editorEntityList_.begin()->second)
            editorEntityList_.erase(editorEntityList_.begin());
        }
        
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size())
            return;

//        if (selection.size() == 1)
//            return;
        for (uint j = 0; j < selection[0].components_.size(); ++j)
        {
            if (selection[0].components_[j]->IsSerializable())
            {
                std::string type = selection[0].components_[j]->TypeName();
                if(type == "EC_Light")
                {
                    EditorEntityList::iterator iter = editorEntityList_.find(type);
                    if (iter == editorEntityList_.end())
                        editorEntityList_[type] = new ECLightEditor(variantManager_, selection[0].components_[j]);
                    //ECLightEditor *editor = new ECLightEditor(variantManager_, selection[0].components_[j]);
                    
                    property_browser_->addProperty(editorEntityList_[type]->GetRootProperty());
                }
            }
        }
    }

    void ECEditorWindow::TogglePropertiesBrowser()
    {
        if (property_browser_)
        {
            if (property_browser_->isVisible())
            {
                property_browser_->hide();
                resize(size().width() - property_browser_->size().width(), size().height());
                if (toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties >"));
            }
            else
            {
                property_browser_->show();
                resize(size().width() + property_browser_->sizeHint().width(), size().height());
                if (toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties <"));
            }
        }
    }

    void ECEditorWindow::PropertyChanged(QtProperty *property)
    {
        EditorEntityList::iterator iter = editorEntityList_.begin();
        while(iter != editorEntityList_.end())
        {
            if (iter->second->ContainProperty(property))
                iter->second->SetPropertyValue(property);
            iter++;
        }
        RevertData();
        //if(property->propertyName)
    }
    
    std::vector<Scene::EntityPtr> ECEditorWindow::GetSelectedEntities()
    {
        std::vector<Scene::EntityPtr> ret;
        
        if (!entity_list_)
            return ret;
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return ret;
        
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

        for (uint i = 0; i < component_list_->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem* item = component_list_->topLevelItem(i);
            if (item->isSelected())
                components.push_back(item->text(0).toStdString());
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

