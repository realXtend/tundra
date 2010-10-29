/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.cpp
 *  @brief  Tree widget showing the scene structure.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SceneTreeWidget.h"
#include "SceneStructureModule.h"

#include "UiServiceInterface.h"
#include "SceneManager.h"
#include "QtUtils.h"
#include "LoggingFunctions.h"
#include "SceneImporter.h"
#include "ComponentManager.h"
#include "ModuleManager.h"
#include "TundraEvents.h"
#include "EventManager.h"
#include "ConfigurationManager.h"
#include "ECEditorWindow.h"
#include "EntityActionDialog.h"
#include "AddComponentDialog.h"
#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "InvokeItem.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneTreeView");

#include <QDomDocument>
#include <QDomElement>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

// File filter definitions for supported files.
const QString cOgreSceneFileFilter(QApplication::translate("SceneTreeWidget", "OGRE scene (*.scene)"));
const QString cOgreMeshFileFilter(QApplication::translate("SceneTreeWidget", "OGRE mesh (*.mesh)"));
const QString cNaaliXmlFileFilter(QApplication::translate("SceneTreeWidget", "Naali scene XML(*.xml)"));
const QString cNaaliBinaryFileFilter(QApplication::translate("SceneTreeWidget", "Naali Binary Format (*.nbf)"));
const QString cAllSupportedTypesFileFilter(QApplication::translate("SceneTreeWidget", "All supported types (*.scene *.mesh *.xml *.nbf)"));

// EntityItem

EntityItem::EntityItem(const Scene::EntityPtr &entity) :
    ptr(entity), id(entity->GetId())
{
}
Scene::EntityPtr EntityItem::Entity() const
{
    return ptr.lock();
}

entity_id_t EntityItem::Id() const
{
    return id;
}

// ComponentItem

ComponentItem::ComponentItem(const ComponentPtr &comp, EntityItem *parent) :
    QTreeWidgetItem(parent), parentItem(parent), ptr(comp), typeName(comp->TypeName()), name(comp->Name())
{
}

ComponentPtr ComponentItem::Component() const
{
    return ptr.lock();
}

EntityItem *ComponentItem::Parent() const
{
    return parentItem;
}

// Selection

bool Selection::IsEmpty() const
{
    return entities.size() == 0 && components.size() == 0;
}

bool Selection::HasEntities() const
{
    return entities.size() > 0;
}

bool Selection::HasComponents() const
{
    return components.size() > 0;
}

QList<entity_id_t> Selection::EntityIds() const
{
    QSet<entity_id_t> ids;
    foreach(EntityItem *e, entities)
        ids.insert(e->Id());
    foreach(ComponentItem *c, components)
        ids.insert(c->Parent()->Id());

    return ids.toList();
}

// SceneTreeWidget

SceneTreeWidget::SceneTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    showComponents(false)
{
    setEditTriggers(/*QAbstractItemView::EditKeyPressed*/QAbstractItemView::NoEditTriggers/*EditKeyPressed*/);
    setDragDropMode(QAbstractItemView::DropOnly/*DragDrop*/);
//    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    //setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setHeaderHidden(true);

    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(Edit()));

    // Create keyboard shortcuts.
    QShortcut *renameShortcut = new QShortcut(QKeySequence(Qt::Key_F2), this);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C), this);
    QShortcut *pasteShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V), this);

    connect(renameShortcut, SIGNAL(activated()), SLOT(Rename()));
    connect(deleteShortcut, SIGNAL(activated()), SLOT(Delete()));
    connect(copyShortcut, SIGNAL(activated()), SLOT(Copy()));
    connect(pasteShortcut, SIGNAL(activated()), SLOT(Paste()));

//    disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnItemEdited(QTreeWidgetItem *, int)));
    LoadInvokeHistory();
}

SceneTreeWidget::~SceneTreeWidget()
{
    if (ecEditor)
        ecEditor->close();
    if (fileDialog)
        fileDialog->close();
}

void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    // Create context menu and show it.
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    AddAvailableActions(menu);

    menu->popup(e->globalPos());
}

void SceneTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->accept();
    else
        e->ignore();
}

void SceneTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->accept();
    else
        e->ignore();
}

void SceneTreeWidget::dropEvent(QDropEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        SceneStructureModule *sceneStruct = framework->GetModule<SceneStructureModule>();
        if (!sceneStruct)
            LogError("Could not retrieve SceneStructureModule. Cannot instantiate content.");

        foreach(QUrl url, data->urls())
        {
            QString filename = url.path();
#ifdef _WINDOWS
            // We have '/' as the first char on windows and the filename
            // is not identified as a file properly. But on other platforms the '/' is valid/required.
            filename = filename.mid(1);
#endif
            if (sceneStruct)
                sceneStruct->InstantiateContent(filename, Vector3df(), false);
        }

        e->acceptProposedAction();
    }
    else
        QTreeWidget::dropEvent(e);
}

void SceneTreeWidget::AddAvailableActions(QMenu *menu)
{
    assert(menu);

    // "New entity...", "Import..." and "Open new scene..." actions are available always
    QAction *newEntityAction = new QAction(tr("New entity..."), menu);
    QAction *importAction = new QAction(tr("Import..."), menu);
    QAction *openNewSceneAction = new QAction(tr("Open new scene..."), menu);

    connect(newEntityAction, SIGNAL(triggered()), SLOT(NewEntity()));
    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    connect(openNewSceneAction, SIGNAL(triggered()), SLOT(OpenNewScene()));

    // "Paste" action is available only if we have valid entity-component XML data in clipboard.
    QAction *pasteAction = 0;
    bool pastePossible = false;
    {
        QDomDocument scene_doc("Scene");
        pastePossible = scene_doc.setContent(QApplication::clipboard()->text());
        if (pastePossible)
        {
            pasteAction = new QAction(tr("Paste"), menu);
            connect(pasteAction, SIGNAL(triggered()), SLOT(Paste()));
        }
    }

    // "Save scene as..." action is possible if we have at least one entity in the scene.
    bool saveSceneAsPossible = (topLevelItemCount() > 0);
    QAction *saveSceneAsAction = 0;
    if (saveSceneAsPossible)
    {
        saveSceneAsAction = new QAction(tr("Save scene as..."), menu);
        connect(saveSceneAsAction, SIGNAL(triggered()), SLOT(SaveSceneAs()));
    }

    // "Edit", "Edit in new", "New component...", "Delete", "Copy", "Actions..." and "Functions..."
    // actions are available only if we have selection.
    QAction *editAction = 0, *editInNewAction = 0, *newComponentAction = 0, *deleteAction = 0,
        *renameAction = 0, *copyAction = 0, *saveAsAction = 0, *actionsAction = 0, *functionsAction = 0;

    bool hasSelection = !selectionModel()->selection().isEmpty();
    if (hasSelection)
    {
        editAction = new QAction(tr("Edit"), menu);
        editInNewAction = new QAction(tr("Edit in new window"), menu);
        newComponentAction = new QAction(tr("New component..."), menu);
        deleteAction = new QAction(tr("Delete"), menu);
        copyAction = new QAction(tr("Copy"), menu);
        saveAsAction = new QAction(tr("Save as..."), menu);
        actionsAction = new QAction(tr("Actions..."), menu);
        functionsAction = new QAction(tr("Functions..."), menu);

        connect(editAction, SIGNAL(triggered()), SLOT(Edit()));
        connect(editInNewAction, SIGNAL(triggered()), SLOT(EditInNew()));
        connect(newComponentAction, SIGNAL(triggered()), SLOT(NewComponent()));
        connect(deleteAction, SIGNAL(triggered()), SLOT(Delete()));
        connect(copyAction, SIGNAL(triggered()), SLOT(Copy()));
        connect(saveAsAction, SIGNAL(triggered()), SLOT(SaveAs()));
        connect(actionsAction, SIGNAL(triggered()), SLOT(OpenEntityActionDialog()));
        connect(functionsAction, SIGNAL(triggered()), SLOT(OpenFunctionDialog()));
    }

    // "Rename" action is possible only if have one entity selected.
    bool renamePossible = (selectionModel()->selection().size() == 1);
    if (renamePossible)
    {
        renameAction = new QAction(tr("Rename"), menu);
        connect(renameAction, SIGNAL(triggered()), SLOT(Rename()));
    }

    if (renamePossible)
        menu->addAction(renameAction);

    if (hasSelection)
    {
        menu->addAction(editAction);
        menu->addAction(editInNewAction);
    }

    menu->addAction(newEntityAction);

    if (hasSelection)
    {
        menu->addAction(newComponentAction);
        menu->addAction(deleteAction);
        menu->addAction(copyAction);
    }

    if (pastePossible)
        menu->addAction(pasteAction);

    menu->addSeparator();

    if (hasSelection)
        menu->addAction(saveAsAction);

    if (saveSceneAsPossible)
        menu->addAction(saveSceneAsAction);

    menu->addAction(importAction);
    menu->addAction(openNewSceneAction);

    if (hasSelection)
    {
        menu->addSeparator();
        menu->addAction(actionsAction);
        menu->addAction(functionsAction);

        // "Functions..." is disabled if we have both entities and components selected simultaneously.
        Selection sel = GetSelection();
        if (sel.HasEntities() && sel.HasComponents())
            functionsAction->setDisabled(true);
    }
/*
    if (invokeHistory.size())
    {
        menu->addSeparator();
        int numberOfInvokeItemsVisible = 5;
        for(int i = 0; i < numberOfInvokeItemsVisible; ++i)
            if (i < invokeHistory.size())
            {
                QAction *invokeAction = new QAction(invokeHistory[i].ToString(), menu);
                menu->addAction(invokeAction);
                connect(invokeAction, SIGNAL(triggered()), SLOT(InvokeActionTriggered()));
                invokeAction->setDisabled(true);
            }
    }
*/
}

void SceneTreeWidget::LoadInvokeHistory()
{
    int idx = 0;
    //framework->GetDefaultConfig().GetSetting<int>("InvokeHistory", "item" + ToString(idx));

    invokeHistory = QList<InvokeItem>();
    for(int i = 0; i< invokeHistory.size(); ++i)
    {
        invokeHistory[i].mruOrder = invokeHistory.size() - 1;
    }
}

void SceneTreeWidget::SaveInvokeHistory()
{
    qSort(invokeHistory);

    Foundation::ConfigurationManager &cfgMgr = framework->GetDefaultConfig();
    for(int idx = 0; idx < invokeHistory.size(); ++idx)
        cfgMgr.SetSetting<std::string>("InvokeHistory", "item" + ToString(idx), invokeHistory[idx].ToSetting());
}

Selection SceneTreeWidget::GetSelection() const
{
    Selection ret;
    QListIterator<QTreeWidgetItem *> it(selectedItems());
    while(it.hasNext())
    {
        QTreeWidgetItem *item = it.next();
        EntityItem *eItem = dynamic_cast<EntityItem *>(item);
        if (eItem)
            ret.entities << eItem;
        else
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(item);
            if (cItem)
                ret.components << cItem;
        }
    }

    return ret;
}

QString SceneTreeWidget::GetSelectionAsXml() const
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return QString();

    // Create root Scene element always for consistency, even if we only have one entity
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");

    if (selection.HasEntities())
    {
        foreach(EntityItem *eItem, selection.entities)
        {
            Scene::EntityPtr entity = eItem->Entity();
            assert(entity.get());
            if (entity)
            {
                QDomElement entity_elem = scene_doc.createElement("entity");
                entity_elem.setAttribute("id", QString::number((int)entity->GetId()));

                foreach(ComponentPtr component, entity->GetComponentVector())
                    if (component->IsSerializable())
                        component->SerializeTo(scene_doc, entity_elem);

                scene_elem.appendChild(entity_elem);
            }
        }

        scene_doc.appendChild(scene_elem);
    }
    else if (selection.HasComponents())
    {
        foreach(ComponentItem *cItem, selection.components)
        {
            ComponentPtr component = cItem->Component();
            if (component && component->IsSerializable())
                component->SerializeTo(scene_doc, scene_elem);
        }

        scene_doc.appendChild(scene_elem);
    }

    return scene_doc.toString();
}

InvokeItem *SceneTreeWidget::FindMruItem() const
{
    InvokeItem *mruItem = 0;

    if (!invokeHistory.empty())
        foreach(InvokeItem ii, invokeHistory)
        {
            if (mruItem == 0)
                mruItem = &ii;
            
            if (ii.mruOrder > mruItem->mruOrder)
                mruItem = &ii;
        }

    return mruItem;
}

void SceneTreeWidget::Edit()
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return;

    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    // If we have an existing editor instance, use it.
    if (ecEditor)
    {
        foreach(entity_id_t id, selection.EntityIds())
            ecEditor->AddEntity(id);
        ecEditor->SetSelectedEntities(selection.EntityIds());
        ui->BringWidgetToFront(ecEditor);
        return;
    }

    // Create new editor.
    ecEditor = new ECEditor::ECEditorWindow(framework);
    ecEditor->setAttribute(Qt::WA_DeleteOnClose);
    ecEditor->move(mapToGlobal(pos()) + QPoint(50, 50));
    ecEditor->hide();
    foreach(entity_id_t id, selection.EntityIds())
        ecEditor->AddEntity(id);
    ecEditor->SetSelectedEntities(selection.EntityIds());

    ui->AddWidgetToScene(ecEditor);
    ui->ShowWidget(ecEditor);
    ui->BringWidgetToFront(ecEditor);
}

void SceneTreeWidget::EditInNew()
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return;

    // Create new editor instance every time, but if our "singleton" editor is not instantiated, create it.
    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    ECEditor::ECEditorWindow *editor = new ECEditor::ECEditorWindow(framework);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->move(mapToGlobal(pos()) + QPoint(50, 50));
    editor->hide();
    foreach(entity_id_t id, selection.EntityIds())
        editor->AddEntity(id);
    editor->SetSelectedEntities(selection.EntityIds());

    if (!ecEditor)
        ecEditor = editor;

    ui->AddWidgetToScene(editor);
    ui->ShowWidget(editor);
    ui->BringWidgetToFront(editor);
}

void SceneTreeWidget::Rename()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    Selection sel = GetSelection();
    if (sel.entities.size() == 1)
    {
        EntityItem *eItem = sel.entities[0];
        Scene::EntityPtr entity = eItem->Entity();
        if (entity)
        {
            // Remove the entity ID from the text when user is editing entity's name.
            eItem->setText(0, entity->GetName());
//            openPersistentEditor(eItem);
//            setCurrentIndex(index);
            edit(index);
            connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(OnItemEdited(QTreeWidgetItem *, int)), Qt::UniqueConnection);
//            connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(CloseEditor(QTreeWidgetItem *,QTreeWidgetItem *)));
        }
    }
/*
    else if (sel.components.size() == 1)
    {
        ComponentItem *cItem = sel.components[0];
        // Remove the type name from the text when user is editing entity's name.
        cItem->setText(0, cItem->name);
        edit(index);
//        connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(OnItemEdited(QTreeWidgetItem *)), Qt::UniqueConnection);
    }
*/
}

void SceneTreeWidget::OnItemEdited(QTreeWidgetItem *item, int column)
{
    if (column != 0)
        return;

    EntityItem *eItem = dynamic_cast<EntityItem *>(item);
    if (eItem)
    {
        Scene::EntityPtr entity = eItem->Entity();
        assert(entity);
        if (entity)
        {
            QString newName = eItem->text(0);
            disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnItemEdited(QTreeWidgetItem *, int)));
            // We don't need to set item text here. It's done when SceneStructureWindow gets AttributeChanged() signal from Scene.
            entity->SetName(newName);
//            closePersistentEditor(item);
            return;
        }
    }
/*
    ComponentItem *cItem = dynamic_cast<ComponentItem *>(item);
    EntityItem *parentItem = dynamic_cast<EntityItem *>(cItem->parent());
    if (cItem && parentItem)
    {
        Scene::EntityPtr entity = scene->GetEntity(parentItem->id);
        QString newName = cItem->text(0);
        disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnItemEdited(QTreeWidgetItem *)));
        ComponentPtr component = entity->GetComponent(cItem->typeName, cItem->name);
        if (component)
            component->SetName(newName);
        //cItem->typeName
    }
*/
}

void SceneTreeWidget::CloseEditor(QTreeWidgetItem *c, QTreeWidgetItem *p)
{
//    foreach(EntityItem *eItem, GetSelection().entities)
    closePersistentEditor(p);
}

void SceneTreeWidget::NewEntity()
{
    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    entity_id_t id;
    AttributeChange::Type changeType;

    // Show a dialog so that user can choose if he wants to create local or synchronized entity.
    QStringList types(QStringList() << tr("Synchronized") << tr("Local"));
    bool ok;
    QString type = QInputDialog::getItem(this, tr("Choose Entity Type"), tr("Type:"), types, 0, false, &ok);
    if (!ok || type.isEmpty())
        return;

    if (type == tr("Synchronized"))
    {
        id = scene->GetNextFreeId();
        changeType = AttributeChange::Replicate;
    }
    else if(type == tr("Local"))
    {
        id =scene->GetNextFreeIdLocal();
        changeType = AttributeChange::LocalOnly;
    }
    else
    {
        LogError("Invalid entity type:" + type.toStdString());
        return;
    }

    // Create entity.
    Scene::EntityPtr entity = scene->CreateEntity(id, QStringList(), changeType);
    assert(entity.get());
    scene->EmitEntityCreated(entity, changeType);
}

void SceneTreeWidget::NewComponent()
{
    Selection sel = GetSelection();
    if (!sel.HasEntities())
        return;

    ECEditor::AddComponentDialog *dialog = new ECEditor::AddComponentDialog(framework, sel.EntityIds(), this);
    dialog->SetComponentList(framework->GetComponentManager()->GetAvailableComponentTypeNames());
    connect(dialog, SIGNAL(finished(int)), SLOT(ComponentDialogFinished(int)));
    dialog->show();
}

void SceneTreeWidget::ComponentDialogFinished(int result)
{
    ECEditor::AddComponentDialog *dialog = qobject_cast<ECEditor::AddComponentDialog *>(sender());
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    if (!scene)
    {
       LogWarning("Fail to add new component to entity, since default world scene was null");
        return;
    }

    QList<entity_id_t> entities = dialog->GetEntityIds();
    for(int i = 0; i < entities.size(); i++)
    {
        Scene::EntityPtr entity = scene->GetEntity(entities[i]);
        if (!entity)
        {
            LogWarning("Fail to add new component to entity, since couldn't find a entity with ID:" + ::ToString<entity_id_t>(entities[i]));
            continue;
        }

        // Check if component has been already added to a entity.
        ComponentPtr comp = entity->GetComponent(dialog->GetTypename(), dialog->GetName());
        if (comp)
        {
            LogWarning("Fail to add a new component, cause there was already a component with a same name and a type");
            continue;
        }

        comp = framework->GetComponentManager()->CreateComponent(dialog->GetTypename(), dialog->GetName());
        assert(comp.get());
        if (comp)
        {
            comp->SetNetworkSyncEnabled(dialog->GetSynchronization());
            entity->AddComponent(comp, AttributeChange::Default);
        }
    }
}

void SceneTreeWidget::Delete()
{
    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    Selection sel = GetSelection();
    // If we have components selected, remove them first.
    if (sel.HasComponents())
        foreach(ComponentItem *cItem, sel.components)
        {
            Scene::EntityPtr entity = cItem->Parent()->Entity();
            ComponentPtr component = cItem->Component();
            if (entity && component)
                entity->RemoveComponent(component, AttributeChange::Default);
        }

    // Remove entities.
    if (sel.HasEntities())
        foreach(entity_id_t id, GetSelection().EntityIds())
            scene->RemoveEntity(id, AttributeChange::Replicate);
}

void SceneTreeWidget::Copy()
{
    QString sceneXml = GetSelectionAsXml();
    if (!sceneXml.isEmpty())
        QApplication::clipboard()->setText(sceneXml);
}

void SceneTreeWidget::Paste()
{
    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    QString errorMsg;
    QDomDocument scene_doc("Scene");
    if (!scene_doc.setContent(QApplication::clipboard()->text(), false, &errorMsg))
    {
        LogError("Parsing scene XML from clipboard failed: " + errorMsg.toStdString());
        return;
    }

    ///\todo Move all code below, except scene->CreateContentFromXml(), to SceneManager.
    QDomElement sceneElem = scene_doc.firstChildElement("scene");
    if (sceneElem.isNull())
        return;

    QDomElement entityElem = sceneElem.firstChildElement("entity");
    if (entityElem.isNull())
    {
        // No entity element, we probably have just components then. Search for component element.
        QDomElement componentElem = sceneElem.firstChildElement("component");
        if (componentElem.isNull())
        {
            LogError("");
            return;
        }

        // Get currently selected entities and paste components to them.
        foreach(EntityItem *eItem, GetSelection().entities)
        {
            Scene::EntityPtr entity = eItem->Entity();
            if (entity)
            {
                while(!componentElem.isNull())
                {
                    QString type = componentElem.attribute("type");
                    QString name = componentElem.attribute("name");
                    if (!type.isNull())
                    {
                        // If we already have component with the same type name and name, add suffix to the new component's name.
                        if (entity->HasComponent(type, name))
                            name.append("_copy");

                        ComponentPtr component = framework->GetComponentManager()->CreateComponent(type, name);
                        if (component)
                        {
                            component->DeserializeFrom(componentElem, AttributeChange::Default);
                            entity->AddComponent(component);
                        }
                    }

                    componentElem = componentElem.nextSiblingElement("component");
                }

                // Rewind back to start if we are pasting components to multiple entities.
                componentElem = sceneElem.firstChildElement("component");
            }
        }
    }

    scene->CreateContentFromXml(scene_doc, false, AttributeChange::Replicate);
}

void SceneTreeWidget::SaveAs()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = Foundation::QtUtils::SaveFileDialogNonModal(cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter,
        tr("Save Selection"), "", 0, this, SLOT(SaveSelectionDialogClosed(int)));
}

void SceneTreeWidget::SaveSceneAs()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = Foundation::QtUtils::SaveFileDialogNonModal(cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter,
        tr("Save Scene"), "", 0, this, SLOT(SaveSceneDialogClosed(int)));
}

void SceneTreeWidget::Import()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = Foundation::QtUtils::OpenFileDialogNonModal(cAllSupportedTypesFileFilter + ";;" +
        cOgreSceneFileFilter + ";;"  + cOgreMeshFileFilter + ";;" + cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter,
        tr("Import"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::OpenNewScene()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = Foundation::QtUtils::OpenFileDialogNonModal(cAllSupportedTypesFileFilter + ";;" +
        cOgreSceneFileFilter + ";;" + cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter,
        tr("Open New Scene"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::OpenEntityActionDialog()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    assert(action);
    if (!action)
        return;

    Selection sel = GetSelection();
    if (!sel.HasEntities())
        return;

    QList<Scene::EntityWeakPtr> entities;
    foreach(EntityItem *eItem, sel.entities)
    {
        Scene::EntityPtr e = eItem->Entity();
        if (e)
            entities.append(e);
    }

    EntityActionDialog *d = new EntityActionDialog(entities, this);
    connect(d, SIGNAL(finished(int)), this, SLOT(EntityActionDialogFinished(int)));
    d->show();
}

void SceneTreeWidget::EntityActionDialogFinished(int result)
{
    EntityActionDialog *dialog = qobject_cast<EntityActionDialog *>(sender());
    if (!dialog)
        return;

    if (result == QDialog::Rejected)
        return;

    EntityAction::ExecutionTypeField execTypes = dialog->ExecutionType();
    QString action = dialog->Action();
    QStringList params = dialog->Parameters();

    foreach(Scene::EntityWeakPtr e, dialog->Entities())
        if (e.lock())
            e.lock()->Exec(execTypes, action, params);

    // Save invoke item
    InvokeItem *mruItem = FindMruItem();

    InvokeItem ii;
    ii.type = Action;
    ii.name = action;
    ii.execTypes = execTypes;
    if (mruItem)
        ii.mruOrder = mruItem->mruOrder + 1;
    else
        ii.mruOrder = 0;

    for(int i = 0; i < params.size(); ++i)
        ii.parameters.push_back(qMakePair(QString("QString"), QVariant(params[i])));

    invokeHistory.push_back(ii);
}

void SceneTreeWidget::OpenFunctionDialog()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    assert(action);
    if (!action)
        return;

    Selection sel = GetSelection();
    if (sel.IsEmpty())
        return;

    QList<boost::weak_ptr<QObject> > objs;
    if (sel.HasEntities())
        foreach(EntityItem *eItem, sel.entities)
        {
            Scene::EntityPtr e = eItem->Entity();
            if (e)
                objs.append(boost::dynamic_pointer_cast<QObject>(e));
        }
    else if(sel.HasComponents())
        foreach(ComponentItem *cItem, sel.components)
        {
            ComponentPtr c = cItem->Component();
            if (c)
                objs.append(boost::dynamic_pointer_cast<QObject>(c));
        }

    FunctionDialog *d = new FunctionDialog(objs, this);
    connect(d, SIGNAL(finished(int)), SLOT(FunctionDialogFinished(int)));
    d->show();
    d->move(300,300);
}

void SceneTreeWidget::FunctionDialogFinished(int result)
{
    FunctionDialog *dialog = qobject_cast<FunctionDialog *>(sender());
    if (!dialog)
        return;

    if (result == QDialog::Rejected)
        return;

    QList<IArgumentType *> arguments = dialog->Arguments();
    foreach(IArgumentType *arg, arguments)
        arg->UpdateValueFromEditor();

    IArgumentType* retValArg = dialog->ReturnValueArgument();
    if (!retValArg)
    {
        LogError("Invalid return value argument. Cannot execute " + dialog->Function().toStdString() + ".");
        return;
    }

    // Clear old return value from the dialog.
    dialog->SetReturnValueText("");

    foreach(boost::weak_ptr<QObject> o, dialog->Objects())
        if (o.lock())
        {
            QObject *obj = o.lock().get();
            QString objName = obj->metaObject()->className();

            QGenericReturnArgument retArg = retValArg->ReturnValue();

            InvokeItem iItem;
            iItem.type = Function;
            iItem.name = dialog->Function();

            QList<QGenericArgument> args;
            for(int i = 0; i < arguments.size(); ++i)
            {
                IArgumentType *arg = arguments[i];
                args.push_back(arg->Value());
                iItem.parameters.push_back(qMakePair(QString(arg->Value().name()), arg->ToQVariant()));
            }

            while(args.size() < 10)
                args.push_back(QGenericArgument());

            if (retValArg->ToString() == "void")
            {
                QMetaObject::invokeMethod(obj, dialog->Function().toStdString().c_str(), Qt::DirectConnection,
                    args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

                dialog->SetReturnValueText(objName + ' ' + retValArg->ToString());
            }
            else
            {
                QMetaObject::invokeMethod(obj, dialog->Function().toStdString().c_str(), Qt::DirectConnection,
                    retArg, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

                retValArg->SetValue(retArg.data());

                LogInfo("Function call returned " + retValArg->ToString().toStdString() + ".");

                dialog->AppendReturnValueText(objName + ' ' + retValArg->ToString());
            }

            // Save invoke item
            InvokeItem *mruItem = FindMruItem();
            if (mruItem)
                iItem.mruOrder = mruItem->mruOrder + 1;
            else
                iItem.mruOrder = 0;
            invokeHistory.push_back(iItem);
        }
}

void SceneTreeWidget::SaveSelectionDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    QStringList files = dialog->selectedFiles();
    if (files.size() != 1)
        return;

    // Check out file extension. If filename has none, use the selected name filter from the save file dialog.
    QString fileExtension;
    if (files[0].lastIndexOf('.') != -1)
    {
        fileExtension = files[0].mid(files[0].lastIndexOf('.'));
    }
    else if (dialog->selectedNameFilter() == cNaaliXmlFileFilter)
    {
        fileExtension = ".xml";
        files[0].append(fileExtension);
    }
    else if (dialog->selectedNameFilter() == cNaaliBinaryFileFilter)
    {
        fileExtension = ".nbf";
        files[0].append(fileExtension);
    }

    QFile file(files[0]);
    if (!file.open(QIODevice::WriteOnly))
    {
        LogError("Could not open file " + files[0].toStdString() + " for writing.");
        return;
    }

    QByteArray bytes;

    if (fileExtension == ".xml")
    {
        bytes = GetSelectionAsXml().toAscii();
    }
    else
    {
        // Handle all other as binary.
        Selection sel = GetSelection();
        if (!sel.IsEmpty())
        {
            // Assume 4MB max for now
            bytes.resize(4 * 1024 * 1024);
            kNet::DataSerializer dest(bytes.data(), bytes.size());

            dest.Add<u32>(sel.entities.size());

            foreach(EntityItem *eItem, sel.entities)
            {
                Scene::EntityPtr entity = eItem->Entity();
                assert(entity.get());
                if (entity)
                    entity->SerializeToBinary(dest);
            }

            bytes.resize(dest.BytesFilled());
        }
    }

    file.write(bytes);
    file.close();
}

void SceneTreeWidget::SaveSceneDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    QStringList files = dialog->selectedFiles();
    if (files.size() != 1)
        return;

    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    // Check out file extension. If filename has none, use the selected name filter from the save file dialog.
    bool binary = false;
    QString fileExtension;
    if (files[0].lastIndexOf('.') != -1)
    {
        fileExtension = files[0].mid(files[0].lastIndexOf('.'));
        if (fileExtension.compare(".xml", Qt::CaseInsensitive) != 0)
            binary = true;
    }
    else if (dialog->selectedNameFilter() == cNaaliXmlFileFilter)
    {
        fileExtension = ".xml";
        files[0].append(fileExtension);
    }
    else if (dialog->selectedNameFilter() == cNaaliBinaryFileFilter)
    {
        fileExtension = ".nbf";
        files[0].append(fileExtension);
        binary = true;
    }

    if (binary)
        scene->SaveSceneBinary(files[0].toStdString());
    else
        scene->SaveSceneXML(files[0].toStdString());
}

void SceneTreeWidget::OpenFileDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != QDialog::Accepted)
        return;

    foreach(QString filename, dialog->selectedFiles())
    {
        bool clearScene = false;
        ///\todo This is awful hack, find better way
        if (dialog->windowTitle() == tr("Open New Scene"))
            clearScene = true;

        SceneStructureModule *sceneStruct = framework->GetModule<SceneStructureModule>();
        if (sceneStruct)
            sceneStruct->InstantiateContent(filename, Vector3df(), clearScene);
        else
            LogError("Could not retrieve SceneStructureModule. Cannot instantiate content.");
    }
}

void SceneTreeWidget::InvokeActionTriggered()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    assert(action);
    if (!action)
        return;

    action->text();
}
