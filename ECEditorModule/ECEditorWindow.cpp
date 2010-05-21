/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.cpp
 *  @brief  Entity-component editor window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "AttributeBrowser.h"

#include "ModuleManager.h"
#include "SceneManager.h"
#include "ComponentManager.h"
#include "XMLUtilities.h"
#include "SceneEvents.h"
#include "EventManager.h"

#include <QUiLoader>

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

    uint AddTreeItem(QTreeWidget *list, const QString &type_name, const QString &name, int entity_id)
    {
        for(int i = 0; i < list->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *existing = list->topLevelItem(i);
            if (existing && existing->text(0) == type_name)
            {
                // We have already item for this EC. Create a dummy parent for the existing EC item and
                // the new one we're adding if it's not already.
                ///\todo Check/test if the code block below is required for real.
                if (existing->text(2) == "(Multiple)")
                {
                    // It's already dummy parent. Add new item to its child.
                    QTreeWidgetItem *item = new QTreeWidgetItem(existing);
                    item->setText(0, type_name);
                    item->setText(1, name);
                    item->setText(2, QString::number(entity_id));
                    existing->addChild(item);
                    return i;
                }

                // The existing item is not dummy parent yet, make it now.
                QTreeWidgetItem *dummyParent = new QTreeWidgetItem(list);
                dummyParent->setText(0, type_name);
                dummyParent->setText(1, "");
                dummyParent->setText(2, "(Multiple)");

                // Relocate the existing item from the top level to a child of the dummy parent.
                existing = list->takeTopLevelItem(i);
                dummyParent->addChild(existing);
                list->addTopLevelItem(dummyParent);

                // Finally, create new item for this EC.
                QTreeWidgetItem *item = new QTreeWidgetItem(dummyParent);
                item->setText(0, type_name);
                item->setText(1, name);
                item->setText(2, QString::number(entity_id));
                dummyParent->addChild(item);
                return i;
            }
        }

        // No existing top level item, create one now.
        QTreeWidgetItem *item = new QTreeWidgetItem(list);
        item->setText(0, type_name);
        item->setText(1, name);
        item->setText(2, QString::number(entity_id));
        list->addTopLevelItem(item);
        return list->topLevelItemCount() - 1;
    }

    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        QWidget(),
        framework_(framework),
        toggle_browser_button_(0),
        entity_list_(0),
        component_list_(0),
        attribute_browser_(0)
    {
        Initialize();
    }

    ECEditorWindow::~ECEditorWindow()
    {
        // Explicitily delete component list widget because it's parent of dynamically allocated items.
        SAFE_DELETE(component_list_);
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

    void ECEditorWindow::RemoveEntity(entity_id_t entity_id)
    {
        if (!entity_list_)
            return;

        for(int i = 0; i < entity_list_->count(); ++i)
        {
            if (entity_list_->item(i)->text() == QString::number(entity_id))
            {
                QListWidgetItem* item = entity_list_->takeItem(i);
                SAFE_DELETE(item);
            }
        }
    }

    void ECEditorWindow::ClearEntities()
    {
        if (entity_list_)
            entity_list_->clear();
        if (component_list_)
            component_list_->clear();
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

    void ECEditorWindow::DeleteComponent()
    {
        if (!component_list_)
            return;

        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        StringVector components;
        for(uint i = 0; i < component_list_->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *item = component_list_->topLevelItem(i);
            if (item->isSelected())
                components.push_back(item->text(0).toStdString());
        }

        for(uint i = 0; i < entities.size(); ++i)
            for(uint j = 0; j < components.size(); ++j)
                entities[i]->RemoveComponent(entities[i]->GetComponent(components[j]), Foundation::Local);

        RefreshEntityComponents();
    }
    
    void ECEditorWindow::CreateComponent()
    {
        bool ok;
        QString name = QInputDialog::getItem(this, tr("Create Component"), tr("Component:"), GetAvailableComponents(), 0, false, &ok);
        if (!ok || name.isEmpty())
            return;

        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        for (uint i = 0; i < entities.size(); ++i)
        {
            // We (mis)use the GetOrCreateComponent function to avoid adding the same EC multiple times, since identifying multiple EC's of similar type
            // is problematic with current API
            Foundation::ComponentInterfacePtr comp = entities[i]->GetOrCreateComponent(name.toStdString(), Foundation::Local);
            if (comp)
            {
                // Trigger change notification in the component so that it updates its initial internal state, if necessary
                comp->ComponentChanged(Foundation::Local);
            }
        }

        RefreshEntityComponents();
    }

    void ECEditorWindow::DeleteEntity()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        std::vector<Scene::EntityPtr> entities = GetSelectedEntities();
        for(uint i = 0; i < entities.size(); ++i)
            scene->RemoveEntity(entities[i]->GetId(), Foundation::Local);
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

        std::vector<QString> added_components;
        for (uint i = 0; i < entities.size(); ++i)
        {
            const Scene::Entity::ComponentVector& components = entities[i]->GetComponentVector();
            for (uint j = 0; j < components.size(); ++j)
            {
                QString component_name = components[j]->TypeName().c_str();
                added_components.push_back(component_name);
                AddTreeItem(component_list_, components[j]->TypeName().c_str(), components[j]->Name().c_str(), entities[i]->GetId());
            }
        }
    }

    void ECEditorWindow::RefreshPropertyBrowser()
    {
        attribute_browser_->ClearBrowser();
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size() && attribute_browser_)
            return;

        for(uint i = 0; i < selection.size(); i++)
        {
            std::vector<Foundation::ComponentInterfacePtr> components = selection[i].components;
            for(uint j = 0; j < components.size(); j++)
                attribute_browser_->AddEntityComponents(components);
        }
    }

    void ECEditorWindow::TogglePropertiesBrowser()
    {
        if (attribute_browser_)
        {
            if (attribute_browser_->isVisible())
            {
                attribute_browser_->hide();
                resize(size().width() - attribute_browser_->size().width(), size().height());
                if (toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties >"));
            }
            else
            {
                attribute_browser_->show();
                resize(size().width() + attribute_browser_->sizeHint().width(), size().height());
                if (toggle_browser_button_)
                    toggle_browser_button_->setText(tr("Properties <"));
            }
        }
    }

    void ECEditorWindow::ShowEntityContextMenu(const QPoint &pos)
    {
        assert(entity_list_);
        if (!entity_list_)
            return;

        QListWidgetItem *item = entity_list_->itemAt(pos);
        if (!item)
            return;

        QMenu *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        QAction *editXml = new QAction(tr("Edit XML..."), menu);
        QAction *deleteEntity= new QAction(tr("Delete"), menu);
        QAction *addComponent = new QAction(tr("Add new component..."), menu);

        connect(editXml, SIGNAL(triggered()), this, SLOT(ShowXmlEditorForEntity()));
        connect(deleteEntity, SIGNAL(triggered()), this, SLOT(DeleteEntity()));
        connect(addComponent, SIGNAL(triggered()), this, SLOT(CreateComponent()));

        menu->addAction(editXml);
        menu->addAction(deleteEntity);
        menu->addAction(addComponent);

        menu->popup(entity_list_->mapToGlobal(pos));
    }

    void ECEditorWindow::ShowComponentContextMenu(const QPoint &pos)
    {
        assert(component_list_);
        if (!component_list_)
            return;

        QTreeWidgetItem *item = component_list_->itemAt(pos);
        if (!item)
            return;

        QMenu *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        QAction *editXml= new QAction(tr("Edit XML..."), menu);
        QAction *deleteComponent= new QAction(tr("Delete"), menu);
        QAction *cutEntity = new QAction(tr("Cut"), menu);
        QAction *copyEntity = new QAction(tr("Copy"), menu);
        QAction *pasteEntity = new QAction(tr("Paste"), menu);

        ///\todo Cut, copy & paste functionality.
        cutEntity->setEnabled(false);
        copyEntity->setEnabled(false);
        pasteEntity->setEnabled(false);

        connect(editXml, SIGNAL(triggered()), this, SLOT(ShowXmlEditorForComponent()));
        connect(deleteComponent, SIGNAL(triggered()), this, SLOT(DeleteComponent()));

        menu->addAction(editXml);
        menu->addAction(deleteComponent);
        menu->addAction(cutEntity);
        menu->addAction(copyEntity);
        menu->addAction(pasteEntity);

        menu->popup(component_list_->mapToGlobal(pos));
    }

    void ECEditorWindow::ShowXmlEditorForEntity()
    {
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size())
            return;

        foreach(EntityComponentSelection ecs, selection)
            emit EditEntityXml(ecs.entity);
    }

    void ECEditorWindow::ShowXmlEditorForComponent()
    {
        std::vector<EntityComponentSelection> selection = GetSelectedComponents();
        if (!selection.size())
            return;

        foreach(EntityComponentSelection ecs, selection)
            foreach(Foundation::ComponentInterfacePtr component, ecs.components)
                emit EditComponentXml(component);
    }

    void ECEditorWindow::hideEvent(QHideEvent* hide_event)
    {
        ClearEntities();
        QWidget::hideEvent(hide_event);
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

    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        QWidget *contents = loader.load(&file, this);
        if (!contents)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
        file.close();

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(contents);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);
        setWindowTitle(contents->windowTitle());
        resize(contents->size());

        toggle_browser_button_ = findChild<QPushButton *>("but_show_properties");
        entity_list_ = findChild<QListWidget*>("list_entities");
        component_list_ = findChild<QTreeWidget*>("list_components");

        attribute_browser_ = new AttributeBrowser(this);
        QVBoxLayout *property_layout = findChild<QVBoxLayout *>("verticalLayout_properties");
        if (property_layout)
            property_layout->addWidget(attribute_browser_);
        attribute_browser_->hide();
/*
        if (component_list_ && attribute_browser_)
            connect(attribute_browser_, SIGNAL(AttributesChanged()), this, SLOT(RefreshComponentXmlData()));
*/
        if (entity_list_)
        {
            entity_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            QShortcut* delete_shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), entity_list_);
            connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshEntityComponents()));
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteEntitiesFromList()));
            connect(entity_list_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowEntityContextMenu(const QPoint &)));
        }

        if (component_list_)
        {
            component_list_->header()->setResizeMode(QHeaderView::ResizeToContents);
            component_list_->setSelectionMode(QAbstractItemView::ExtendedSelection);
//            connect(component_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshComponentXmlData()));
            connect(component_list_, SIGNAL(itemSelectionChanged()), this, SLOT(RefreshPropertyBrowser()));
            connect(component_list_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowComponentContextMenu(const QPoint &)));
        }

        if (toggle_browser_button_)
            connect(toggle_browser_button_, SIGNAL(pressed()), this, SLOT(TogglePropertiesBrowser()));
    }

    QStringList ECEditorWindow::GetAvailableComponents() const
    {
        QStringList components;
        Foundation::ComponentManagerPtr comp_mgr = framework_->GetComponentManager();
        const Foundation::ComponentManager::ComponentFactoryMap& factories = comp_mgr->GetComponentFactoryMap();
        Foundation::ComponentManager::ComponentFactoryMap::const_iterator i = factories.begin();
        while (i != factories.end())
        {
            components << i->first.c_str();
            ++i;
        }

        return components;
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
            selection.entity = entities[i];
            for (uint j = 0; j < components.size(); ++j)
            {
                Foundation::ComponentInterfacePtr comp = selection.entity->GetComponent(components[j]);
                if (comp)
                    selection.components.push_back(comp);
            }
            
            ret.push_back(selection);
        }
        
        return ret;
    }
}

