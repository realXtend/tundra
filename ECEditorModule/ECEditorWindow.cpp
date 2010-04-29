// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorWindow.h"
#include "ECEditorModule.h"

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

#include "MemoryLeakCheck.h"

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

        /*QWidget *w = new QWidget();

        QtBoolPropertyManager *boolManager = new QtBoolPropertyManager(w);
        QtIntPropertyManager *intManager = new QtIntPropertyManager(w);
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(w);
        QtSizePropertyManager *sizeManager = new QtSizePropertyManager(w);
        QtRectPropertyManager *rectManager = new QtRectPropertyManager(w);
        QtSizePolicyPropertyManager *sizePolicyManager = new QtSizePolicyPropertyManager(w);
        QtEnumPropertyManager *enumManager = new QtEnumPropertyManager(w);
        QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(w);*/

        //QtProperty *item0 = groupManager->addProperty("QObject");
        //QtProperty *item1 = stringManager->addProperty("objectName");
        /*item0->addSubProperty(item1);

        QtProperty *item2 = boolManager->addProperty("enabled");
        item0->addSubProperty(item2);

        QtProperty *item3 = rectManager->addProperty("geometry");
        item0->addSubProperty(item3);

        QtProperty *item4 = sizePolicyManager->addProperty("sizePolicy");
        item0->addSubProperty(item4);

        QtProperty *item5 = sizeManager->addProperty("sizeIncrement");
        item0->addSubProperty(item5);

        QtProperty *item7 = boolManager->addProperty("mouseTracking");
        item0->addSubProperty(item7);

        QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(w);
        QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(w);
        QtSliderFactory *sliderFactory = new QtSliderFactory(w);
        QtScrollBarFactory *scrollBarFactory = new QtScrollBarFactory(w);
        QtLineEditFactory *lineEditFactory = new QtLineEditFactory(w);
        QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(w);*/

        property_browser_ = new QtTreePropertyBrowser();
        QVBoxLayout *property_layout = findChild<QVBoxLayout *>("verticalLayout_properties");
        property_layout->addWidget(property_browser_);
        property_browser_->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        property_browser_->hide();

        /*property_browser_->setFactoryForManager(boolManager, checkBoxFactory);
        property_browser_->setFactoryForManager(intManager, spinBoxFactory);
        property_browser_->setFactoryForManager(stringManager, lineEditFactory);
        property_browser_->setFactoryForManager(sizeManager->subIntPropertyManager(), spinBoxFactory);
        property_browser_->setFactoryForManager(rectManager->subIntPropertyManager(), spinBoxFactory);
        property_browser_->setFactoryForManager(sizePolicyManager->subIntPropertyManager(), spinBoxFactory);
        property_browser_->setFactoryForManager(sizePolicyManager->subEnumPropertyManager(), comboBoxFactory);
        property_browser_->setFactoryForManager(enumManager, comboBoxFactory);
        
        property_browser_->addProperty(item0);*/

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
            
            Scene::Events::SceneEventData event_data(entities[i]->GetId());
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_ECS_MODIFIED, &event_data);
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
            
            Scene::Events::SceneEventData event_data(entities[i]->GetId());
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_ECS_MODIFIED, &event_data);
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
                            comp->DeserializeFrom(comp_elem, Foundation::Local);
                        comp_elem = comp_elem.nextSiblingElement("component");
                    }
                    
                    Scene::Events::SceneEventData event_data(entity->GetId());
                    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
                    event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_ECS_MODIFIED, &event_data);
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
        
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size())
            return;

        if (selection.size() == 1)
        {
            // Single entity
            //QDomElement entity_elem = temp_doc.createElement("entity");
            //temp_doc.appendChild(entity_elem);
            /*QWidget *widget = new QWidget();

            QtVariantPropertyManager *variantManager = new QtVariantPropertyManager();
            QtProperty *topItem = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Entity"));

            QtVariantProperty *item = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), QString("EnumProperty"));
            QStringList enumNames;
            enumNames << "Point" << "Spot" << "Directional";
            item->setAttribute(QString("enumNames"), enumNames);
            item->setValue(1);
            topItem->addSubProperty(item);

            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
            property_browser_->setFactoryForManager(variantManager, variantFactory);

            QtBoolPropertyManager *boolManager = new QtBoolPropertyManager(widget);
            QtIntPropertyManager *intManager = new QtIntPropertyManager(widget);
            QtStringPropertyManager *stringManager = new QtStringPropertyManager(widget);
            QtSizePropertyManager *sizeManager = new QtSizePropertyManager(widget);
            QtRectPropertyManager *rectManager = new QtRectPropertyManager(widget);
            QtSizePolicyPropertyManager *sizePolicyManager = new QtSizePolicyPropertyManager(widget);
            QtEnumPropertyManager *enumManager = new QtEnumPropertyManager(widget);
            QtColorPropertyManager *colorManager = new QtColorPropertyManager(widget);
            QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(widget);

            QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(widget);
            QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(widget);
            QtSliderFactory *sliderFactory = new QtSliderFactory(widget);
            QtScrollBarFactory *scrollBarFactory = new QtScrollBarFactory(widget);
            QtLineEditFactory *lineEditFactory = new QtLineEditFactory(widget);
            QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(widget);
            QtColorEditorFactory *colorFactory = new QtColorEditorFactory(widget);

            property_browser_->setFactoryForManager(boolManager, checkBoxFactory);
            property_browser_->setFactoryForManager(intManager, spinBoxFactory);
            property_browser_->setFactoryForManager(stringManager, lineEditFactory);
            property_browser_->setFactoryForManager(sizeManager->subIntPropertyManager(), spinBoxFactory);
            property_browser_->setFactoryForManager(rectManager->subIntPropertyManager(), spinBoxFactory);
            property_browser_->setFactoryForManager(sizePolicyManager->subIntPropertyManager(), spinBoxFactory);
            property_browser_->setFactoryForManager(sizePolicyManager->subEnumPropertyManager(), comboBoxFactory);
            property_browser_->setFactoryForManager(enumManager, comboBoxFactory);
            property_browser_->setFactoryForManager(colorManager, colorFactory);

            std::string entity_id = "Entity Id: " + ToString<unsigned int>(selection[0].entity_->GetId()); 
            QtProperty *item0 = groupManager->addProperty(entity_id.c_str());
            for (uint j = 0; j < selection[0].components_.size(); ++j)
            {
                if (selection[0].components_[j]->IsSerializable())
                {
                    std::string type = selection[0].components_[j]->TypeName();
                    if(type == "EC_Light")
                    {
                        QtProperty *lightTypeProperty = enumManager->addProperty("Light type");
                        QStringList enumNames;
                        enumNames << "Point" << "Spot" << "Directional";
                        enumManager->setEnumNames(lightTypeProperty, enumNames);
                        item0->addSubProperty(lightTypeProperty);
                        item0->addSubProperty(lightTypeProperty);
                        item0->addSubProperty(lightTypeProperty);
                    }
                    //selection[0].components_[j]->SerializeTo(temp_doc, entity_elem);
                }
            }

            property_browser_->addProperty(topItem);
            //QtProperty *item1 = stringManager->addProperty("objectName");
            
            QString id_str;
            id_str.setNum((int)selection[0].entity_->GetId());
            entity_elem.setAttribute("id", id_str);
            for (uint j = 0; j < selection[0].components_.size(); ++j)
            {
                if (selection[0].components_[j]->IsSerializable())
                    selection[0].components_[j]->SerializeTo(temp_doc, entity_elem);
            }
            temp_doc.appendChild(entity_elem);*/
        }
    }

    void ECEditorWindow::TogglePropertiesBrowser()
    {
        if(property_browser_)
        {
            if(property_browser_->isVisible())
            {
                property_browser_->hide();
                resize(size().width() - property_browser_->size().width(), size().height());
                if(toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties >"));
            }
            else
            {
                property_browser_->show();
                resize(size().width() + property_browser_->sizeHint().width(), size().height());
                if(toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties <"));
            }
        }
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

