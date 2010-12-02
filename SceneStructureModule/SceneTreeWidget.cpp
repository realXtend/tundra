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
#include "SupportedFileTypes.h"

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
#include "FunctionInvoker.h"
#include "AssetEvents.h"
#include "RexTypes.h"
#include "OgreConversionUtils.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"
#include "ResourceInterface.h"
#include "NaaliUi.h"
#include "NaaliMainWindow.h"
#ifdef OGREASSETEDITOR_ENABLED
#   include "MeshPreviewEditor.h"
#   include "TexturePreviewEditor.h"
#   include "OgreScriptEditor.h"
#endif

DEFINE_POCO_LOGGING_FUNCTIONS("SceneTreeView");

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

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

// AssetItem

AssetItem::AssetItem(const QString &name, const QString &ref, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent)
{
    this->name = name;
    this->id = ref;
}

// Selection

bool Selection::IsEmpty() const
{
    return entities.size() == 0 && components.size() == 0 && assets.isEmpty();
}

bool Selection::HasEntities() const
{
    return entities.size() > 0;
}

bool Selection::HasComponents() const
{
    return components.size() > 0;
}

bool Selection::HasAssets() const
{
    return !assets.isEmpty();
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

// Menu
Menu::Menu(QWidget *parent) : QMenu(parent), shiftDown(false)
{
}

void Menu::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Shift)
        shiftDown = true;
    QWidget::keyPressEvent(e);
}

void Menu::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Shift)
        shiftDown = false;
    QMenu::keyReleaseEvent(e);
}

// SceneTreeWidget

SceneTreeWidget::SceneTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    showComponents(false),
    historyMaxItemCount(100),
    numberOfInvokeItemsVisible(5),
    fetch_references_(false)
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

    SaveInvokeHistory();
}

void SceneTreeWidget::SetScene(const Scene::ScenePtr &s)
{
    scene = s;
}

void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    // Create context menu and show it.
    SAFE_DELETE(contextMenu);
    contextMenu = new Menu(this);

    AddAvailableActions(contextMenu);

    contextMenu->popup(e->globalPos());
}

void SceneTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        foreach (QUrl url, e->mimeData()->urls())
            if (SceneStructureModule::IsSupportedFileType(url.path()))
                e->accept();
    }
    else
        QWidget::dragEnterEvent(e);
}

void SceneTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        foreach(QUrl url, e->mimeData()->urls())
            if (SceneStructureModule::IsSupportedFileType(url.path()))
                e->accept();
    }
    else
        QWidget::dragMoveEvent(e);
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
            if (SceneStructureModule::IsSupportedFileType(filename))
                if (sceneStruct)
                    sceneStruct->InstantiateContent(filename, Vector3df(), false, true);
        }

        e->acceptProposedAction();
    }
    else
        QTreeWidget::dropEvent(e);
}

void SceneTreeWidget::AddAvailableActions(QMenu *menu)
{
    assert(menu);

    Selection sel = GetSelection();

    if (sel.HasAssets() && !sel.HasComponents() && !sel.HasEntities())
        AddAvailableAssetActions(menu);
    else
        AddAvailableEntityActions(menu);
}

void SceneTreeWidget::AddAvailableAssetActions(QMenu *menu)
{
    assert(menu);

    Selection sel = GetSelection();

    QAction *action;
#ifdef OGREASSETEDITOR_ENABLED
    action = new QAction(tr("Edit"), menu);
    connect(action, SIGNAL(triggered()), SLOT(Edit()));
    menu->addAction(action);
    menu->setDefaultAction(action);
#endif

    if (sel.assets.size() > 0)
    {
        if (sel.assets.size() > 1)
            action = new QAction(tr("Save selected assets..."), menu);
        else
        {
            QString assetName = sel.assets[0]->id.right(sel.assets[0]->id.size() - sel.assets[0]->id.lastIndexOf("://") - 3);
            action = new QAction(tr("Save") + " " + assetName + " " + tr("as..."), menu);
        }
        connect(action, SIGNAL(triggered()), SLOT(SaveAssetAs()));
        menu->addAction(action);
    }
}

void SceneTreeWidget::AddAvailableEntityActions(QMenu *menu)
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
    QAction *exportAllAction = 0;
    if (saveSceneAsPossible)
    {
        saveSceneAsAction = new QAction(tr("Save scene as..."), menu);
        connect(saveSceneAsAction, SIGNAL(triggered()), SLOT(SaveSceneAs()));

        exportAllAction = new QAction(tr("Export..."), menu);
        connect(exportAllAction, SIGNAL(triggered()), SLOT(ExportAll()));
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
        menu->setDefaultAction(editAction);
        menu->setDefaultAction(editAction);
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

    if (exportAllAction)
        menu->addAction(exportAllAction);

    menu->addAction(importAction);
    menu->addAction(openNewSceneAction);

    if (hasSelection)
    {
        menu->addSeparator();
        menu->addAction(actionsAction);
        menu->addAction(functionsAction);

        Selection sel = GetSelection();

        // "Functions..." is disabled if we have both entities and components selected simultaneously.
        if (sel.HasEntities() && sel.HasComponents())
            functionsAction->setDisabled(true);

        // Show only function and actions that for possible whole selection.
        QSet<QString> objectNames;
        foreach(EntityItem *eItem, sel.entities)
            if (eItem->Entity())
                objectNames << eItem->Entity().get()->metaObject()->className();
        foreach(ComponentItem *cItem, sel.components)
            if (cItem->Component())
                objectNames << cItem->Component().get()->metaObject()->className();

        // If we have more than one object name in the object name set, don't show the history.
        if (objectNames.size() == 1 && invokeHistory.size())
        {
            menu->addSeparator();

            int numItemsAdded = 0;
            for(int i = 0; i < invokeHistory.size(); ++i)
                if ((invokeHistory[i].objectName == objectNames.toList().first()) && (numItemsAdded < numberOfInvokeItemsVisible))
                {
                    QAction *invokeAction = new QAction(invokeHistory[i].ToString(), menu);
                    menu->addAction(invokeAction);
                    connect(invokeAction, SIGNAL(triggered()), SLOT(InvokeActionTriggered()));
                    ++numItemsAdded;
                }
        }
    }
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
            else
            {
                AssetItem *aItem = dynamic_cast<AssetItem *>(item);
                if (aItem)
                    ret.assets << aItem;
            }
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

void SceneTreeWidget::LoadInvokeHistory()
{
    invokeHistory.clear();

    // Load and parse invoke history from settings.
    ConfigurationManager &cfgMgr = framework->GetDefaultConfig();

    try
    {
        int idx = 0;
        forever
        {
            std::string setting = cfgMgr.GetSetting<std::string>("InvokeHistory", "item" + ToString(idx));
            if (setting.empty())
                break;
            invokeHistory.push_back(InvokeItem(setting));
            ++idx;
        }

        // Set MRU order numbers.
        for(int i = 0; i < invokeHistory.size(); ++i)
            invokeHistory[i].mruOrder = invokeHistory.size() - i;
    }
    catch(const Exception &e)
    {
        LogError(std::string(e.what()));
    }
    catch(...)
    {
    }
}

void SceneTreeWidget::SaveInvokeHistory()
{
    // Sort descending by MRU order.
    qSort(invokeHistory);

    ConfigurationManager &cfgMgr = framework->GetDefaultConfig();
    for(int idx = 0; idx < invokeHistory.size(); ++idx)
        cfgMgr.SetSetting<std::string>("InvokeHistory", "item" + ToString(idx), invokeHistory[idx].ToSetting());
}

InvokeItem *SceneTreeWidget::FindMruItem() const
{
    InvokeItem *mruItem = 0;

    foreach(InvokeItem ii, invokeHistory)
    {
        if (mruItem == 0)
            mruItem = &ii;
        if (ii > *mruItem)
            mruItem = &ii;
    }

    return mruItem;
}

void SceneTreeWidget::Edit()
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return;

    if (selection.HasComponents() || selection.HasEntities())
    {
        //UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
        //assert(ui);

        // If we have an existing editor instance, use it.
        if (ecEditor)
        {
            ecEditor->AddEntities(selection.EntityIds(), true);
            /*foreach(entity_id_t id, selection.EntityIds())
                ecEditor->AddEntity(id, false);
            ecEditor->SetSelectedEntities(selection.EntityIds());*/
            ecEditor->show();
            //ui->BringWidgetToFront(ecEditor);
            return;
        }

        // Create new editor.
        ecEditor = new ECEditor::ECEditorWindow(framework);
        ecEditor->setAttribute(Qt::WA_DeleteOnClose);
        ecEditor->move(mapToGlobal(pos()) + QPoint(50, 50));
        ecEditor->hide();
        ecEditor->AddEntities(selection.EntityIds(), true);
        /*foreach(entity_id_t id, selection.EntityIds())
            ecEditor->AddEntity(id, false);
        ecEditor->SetSelectedEntities(selection.EntityIds());*/

        NaaliUi *ui = framework->Ui();
        if (!ui)
            return;
        ecEditor->setParent(ui->MainWindow());
        ecEditor->setWindowFlags(Qt::Tool);
        ecEditor->show();

        /*ui->AddWidgetToScene(ecEditor);
        ui->ShowWidget(ecEditor);
        ui->BringWidgetToFront(ecEditor);*/ 
    } else
    {
#ifdef OGREASSETEDITOR_ENABLED
        foreach(AssetItem *aItem, selection.assets)
        {
            //int itype = RexTypes::GetAssetTypeFromFilename(aItem->id.toStdString());
            QString type = GetResourceTypeFromResourceFileName(aItem->id.toLatin1());

            if (type == "OgreMesh")
            {
                MeshPreviewEditor::OpenMeshPreviewEditor(framework, QString(OgreRenderer::SanitateAssetIdForOgre(aItem->id.toStdString()).c_str()), aItem->type);
            } else if (type ==  "OgreTexture")
            {
                TexturePreviewEditor::OpenPreviewEditor(framework, QString(OgreRenderer::SanitateAssetIdForOgre(aItem->id.toStdString()).c_str()));
            } else if (type == "OgreMaterial")
            {
                OgreScriptEditor::OpenOgreScriptEditor(framework, QString(OgreRenderer::SanitateAssetIdForOgre(aItem->id.toStdString()).c_str()), RexTypes::RexAT_MaterialScript);
            }
        }
#endif
    }
}

void SceneTreeWidget::EditInNew()
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return;

    // Create new editor instance every time, but if our "singleton" editor is not instantiated, create it.
    //UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    //assert(ui);

    ECEditor::ECEditorWindow *editor = new ECEditor::ECEditorWindow(framework);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    //editor->move(mapToGlobal(pos()) + QPoint(50, 50));
    editor->hide();
    editor->AddEntities(selection.EntityIds(), true);
    /*foreach(entity_id_t id, selection.EntityIds())
        editor->AddEntity(id);
    editor->SetSelectedEntities(selection.EntityIds());*/

    NaaliUi *ui = framework->Ui();
    if (!ui)
        return;
    editor->setParent(ui->MainWindow());
    editor->setWindowFlags(Qt::Tool);
    editor->show();

    if (!ecEditor)
        ecEditor = editor;
    
    /*ui->AddWidgetToScene(editor); 
    ui->ShowWidget(editor);
    ui->BringWidgetToFront(editor);*/
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
    if (scene.expired())
        return;

    entity_id_t id;
    AttributeChange::Type changeType;

    // Show a dialog so that user can choose if he wants to create local or synchronized entity.
    QStringList types(QStringList() << tr("Synchronized") << tr("Local"));
    bool ok;
    QString type = QInputDialog::getItem(NULL, tr("Choose Entity Type"), tr("Type:"), types, 0, false, &ok);
    if (!ok || type.isEmpty())
        return;

    if (type == tr("Synchronized"))
    {
        id = scene.lock()->GetNextFreeId();
        changeType = AttributeChange::Replicate;
    }
    else if(type == tr("Local"))
    {
        id = scene.lock()->GetNextFreeIdLocal();
        changeType = AttributeChange::LocalOnly;
    }
    else
    {
        LogError("Invalid entity type:" + type.toStdString());
        return;
    }

    // Create entity.
    Scene::EntityPtr entity = scene.lock()->CreateEntity(id, QStringList(), changeType);
    assert(entity.get());
    scene.lock()->EmitEntityCreated(entity, changeType);
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

    if (scene.expired())
    {
       LogWarning("Fail to add new component to entity, since default world scene was null");
        return;
    }

    QList<entity_id_t> entities = dialog->GetEntityIds();
    for(int i = 0; i < entities.size(); i++)
    {
        Scene::EntityPtr entity = scene.lock()->GetEntity(entities[i]);
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
    if (scene.expired())
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
            scene.lock()->RemoveEntity(id, AttributeChange::Replicate);
}

void SceneTreeWidget::Copy()
{
    QString sceneXml = GetSelectionAsXml();
    if (!sceneXml.isEmpty())
        QApplication::clipboard()->setText(sceneXml);
}

void SceneTreeWidget::Paste()
{
    if (scene.expired())
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

    scene.lock()->CreateContentFromXml(scene_doc, false, AttributeChange::Replicate);
}

void SceneTreeWidget::SaveAs()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = QtUtils::SaveFileDialogNonModal(cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter,
        tr("Save Selection"), "", 0, this, SLOT(SaveSelectionDialogClosed(int)));
}

void SceneTreeWidget::SaveSceneAs()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = QtUtils::SaveFileDialogNonModal(cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter,
        tr("Save Scene"), "", 0, this, SLOT(SaveSceneDialogClosed(int)));
}

void SceneTreeWidget::ExportAll()
{
    if (fileDialog)
        fileDialog->close();

    if (GetSelection().HasEntities())
    {
        // Save only selected entities
        fileDialog = QtUtils::SaveFileDialogNonModal(cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter,
            tr("Export scene"), "", 0, this, SLOT(SaveSelectionDialogClosed(int)));
    }
    else
    {
        // Save all entities in the scene
        fileDialog = QtUtils::SaveFileDialogNonModal(cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter,
            tr("Export scene"), "", 0, this, SLOT(SaveSceneDialogClosed(int)));
    }

    // Finally export assets
    connect(fileDialog, SIGNAL(finished(int)), this, SLOT(ExportAllDialogClosed(int)));
}

void SceneTreeWidget::Import()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = QtUtils::OpenFileDialogNonModal(cAllSupportedTypesFileFilter + ";;" +
        cOgreSceneFileFilter + ";;"  + cOgreMeshFileFilter + ";;" + 
#ifdef ASSIMP_ENABLED
        cMeshFileFilter + ";;" + 
#endif
        cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter + ";;" + cAllTypesFileFilter,
        tr("Import"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::OpenNewScene()
{
    if (fileDialog)
        fileDialog->close();
    fileDialog = QtUtils::OpenFileDialogNonModal(cAllSupportedTypesFileFilter + ";;" +
        cOgreSceneFileFilter + ";;" + cTundraXmlFileFilter + ";;" + cTundraBinaryFileFilter + ";;" +
        cAllTypesFileFilter, tr("Open New Scene"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::OpenEntityActionDialog()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    assert(action);
    if (!action)
        return;

    Selection sel = GetSelection();
    if (sel.IsEmpty())
        return;

    if (scene.expired())
        return;

    QList<Scene::EntityWeakPtr> entities;
    foreach(entity_id_t id, sel.EntityIds())
    {
        Scene::EntityPtr e = scene.lock()->GetEntity(id);
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
    InvokeItem ii;
    ii.type = InvokeItem::Action;
    ii.objectName = QString(Scene::Entity::staticMetaObject.className());
    ii.name = action;
    ii.execTypes = execTypes;
    InvokeItem *mruItem = FindMruItem();
    ii.mruOrder = mruItem ? mruItem->mruOrder + 1 : 0;

    for(int i = 0; i < params.size(); ++i)
        ii.parameters.push_back(QVariant(params[i]));

    // Do not save duplicates and make sure that history size stays withing max size.
    QList<InvokeItem>::const_iterator it = qFind(invokeHistory, ii);
    if (it == invokeHistory.end())
    {
        while(invokeHistory.size() > historyMaxItemCount - 1)
            invokeHistory.pop_back();

        invokeHistory.push_front(ii);
    }
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

    QObjectWeakPtrList objs;
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

    // Get the list of parameters we will pass to the function we are invoking,
    // and update the latest values to them from the editor widgets the user inputted.
    QVariantList params;
    foreach(IArgumentType *arg, dialog->Arguments())
    {
        arg->UpdateValueFromEditor();
        params << arg->ToQVariant();
    }

    // Clear old return value from the dialog.
    dialog->SetReturnValueText("");

    foreach(QObjectWeakPtr o, dialog->Objects())
        if (o.lock())
        {
            QObject *obj = o.lock().get();

            QString objName = obj->metaObject()->className();
            QString objNameWithId = objName;
            {
                Scene::Entity *e = dynamic_cast<Scene::Entity *>(obj);
                IComponent *c = dynamic_cast<IComponent *>(obj);
                if (e)
                    objNameWithId.append('(' + QString::number((uint)e->GetId()) + ')');
                else if (c)
                    objNameWithId.append('(' + c->Name() + ')');
            }

            // Invoke function.
            QString errorMsg;
            QVariant ret;
            FunctionInvoker invoker;
            invoker.Invoke(obj, dialog->Function(), &ret, params, &errorMsg);

            QString retValStr;
            ///\todo For some reason QVariant::toString() cannot convert QStringList to QString properly.
            /// Convert it manually here.
            if (ret.type() == QVariant::StringList)
                foreach(QString s, ret.toStringList())
                    retValStr.append("\n" + s);

            if (errorMsg.isEmpty())
                dialog->AppendReturnValueText(objNameWithId + " " + retValStr);
            else
                dialog->AppendReturnValueText(objNameWithId + " " + errorMsg);

            // Save invoke item
            InvokeItem ii;
            ii.type = InvokeItem::Function;
            ii.parameters = params;
            ii.name = dialog->Function();
            ii.returnType = (ret.type() == QVariant::Invalid) ? QString("void") : QString(ret.typeName());
            ii.objectName = objName;
            InvokeItem *mruItem = FindMruItem();
            ii.mruOrder = mruItem ? mruItem->mruOrder + 1 : 0;

            // Do not save duplicates and make sure that history size stays withing max size.
            QList<InvokeItem>::const_iterator it = qFind(invokeHistory, ii);
            if (it == invokeHistory.end())
            {
                while(invokeHistory.size() > historyMaxItemCount - 1)
                    invokeHistory.pop_back();

                invokeHistory.push_front(ii);
            }
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
    else if (dialog->selectedNameFilter() == cTundraXmlFileFilter)
    {
        fileExtension = cTundraXmlFileExtension;
        files[0].append(fileExtension);
    }
    else if (dialog->selectedNameFilter() == cTundraBinaryFileFilter)
    {
        fileExtension = cTundraBinFileExtension;
        files[0].append(fileExtension);
    }

    QFile file(files[0]);
    if (!file.open(QIODevice::WriteOnly))
    {
        LogError("Could not open file " + files[0].toStdString() + " for writing.");
        return;
    }

    QByteArray bytes;

    if (fileExtension == cTundraXmlFileExtension)
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

    if (scene.expired())
        return;

    // Check out file extension. If filename has none, use the selected name filter from the save file dialog.
    bool binary = false;
    QString fileExtension;
    if (files[0].lastIndexOf('.') != -1)
    {
        fileExtension = files[0].mid(files[0].lastIndexOf('.'));
        if (fileExtension.compare(cTundraXmlFileExtension, Qt::CaseInsensitive) != 0)
            binary = true;
    }
    else if (dialog->selectedNameFilter() == cTundraXmlFileFilter)
    {
        fileExtension = cTundraXmlFileExtension;
        files[0].append(fileExtension);
    }
    else if (dialog->selectedNameFilter() == cTundraBinaryFileFilter)
    {
        fileExtension = cTundraBinFileExtension;
        files[0].append(fileExtension);
        binary = true;
    }

    if (binary)
        scene.lock()->SaveSceneBinary(files[0].toStdString());
    else
        scene.lock()->SaveSceneXML(files[0].toStdString());
}

void SceneTreeWidget::ExportAllDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);

    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().size() != 1 || scene.expired())
        return;

    // separate path from filename
    QFileInfo fi(dialog->selectedFiles()[0]);
    QDir directory = fi.absoluteDir();
    if (!directory.exists())
        return;


    QSet<QString> assets;
    Selection sel = GetSelection();
    if (!sel.HasEntities())
    {
        // Export all assets
        for (int i = 0; i < topLevelItemCount(); ++i)
        {
            EntityItem *eItem = dynamic_cast<EntityItem *>(topLevelItem(i));
            if (!eItem)
                continue;

            assets.unite(GetAssetRefs(eItem));
        }
    }
    else
    {
        // Export assets for selected entities
        foreach (EntityItem *eItem, sel.entities)
        {
            assets.unite(GetAssetRefs(eItem));
        }
    }

    saved_assets_.clear();
    fetch_references_ = true;
    //! \todo This is in theory a better way to get all assets in a sceene, but not all assets are currently available with this method
    //!       Once all assets are properly shown in this widget, it would be better to do it this way -cm
    /*QTreeWidgetItemIterator it(this);
    while (*it)
    {
        AssetItem *aItem = dynamic_cast<AssetItem*>((*it));
        if (aItem)
        {
            assets.insert(aItem->id);
        }
        ++it;
    }*/


    foreach(const QString &assetid, assets)
    {
        AssetTransferPtr transfer = framework->Asset()->RequestAsset(assetid);

        QString filename = directory.absolutePath();
        QString assetName = assetid.right(assetid.size() - assetid.lastIndexOf("://") - 3);
        filename += QDir::separator() + assetName;

        filesaves_.insert(transfer, filename);
        connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(AssetLoaded(IAssetTransfer *)));
    }
}

QSet<QString> SceneTreeWidget::GetAssetRefs(const EntityItem *eItem) const
{
    assert(scene.lock());
    QSet<QString> assets;

    Scene::EntityPtr entity = scene.lock()->GetEntity(eItem->Id());
    if (entity)
    {
        int entityChildCount = eItem->childCount();
        for(int j = 0; j < entityChildCount; ++j)
        {
            ComponentItem *cItem = dynamic_cast<ComponentItem *>(eItem->child(j));
            if (!cItem)
                continue;

            ComponentPtr comp = entity->GetComponent(cItem->typeName, cItem->name);
            if (!comp)
                continue;

            foreach(ComponentPtr comp, entity->GetComponentVector())
                foreach(IAttribute *attr, comp->GetAttributes())
                    if (attr->TypenameToString() == "assetreference")
                    {
                        Attribute<AssetReference> *assetRef = dynamic_cast<Attribute<AssetReference> *>(attr);
                        if (assetRef)
                            assets.insert(assetRef->Get().ref);
                    }
        }
    }
    return assets;
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
            sceneStruct->InstantiateContent(filename, Vector3df(), clearScene, true);
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

    Selection sel = GetSelection();
    if (sel.IsEmpty())
        return;

    InvokeItem *invokedItem = 0;
    foreach(InvokeItem ii, invokeHistory)
        if (ii.ToString() == action->text())
        {
            invokedItem = &ii;
            break;
        }

    InvokeItem *mruItem = FindMruItem();
    assert(mruItem);
    assert(invokedItem);
    invokedItem->mruOrder = mruItem->mruOrder + 1;

    // Gather target objects.
    QList<Scene::EntityWeakPtr> entities;
    QObjectList objects;
    QObjectWeakPtrList objectPtrs;
    foreach(EntityItem *eItem, sel.entities)
        if (eItem->Entity())
        {
            entities << eItem->Entity();
            objects << eItem->Entity().get();
            objectPtrs << boost::dynamic_pointer_cast<QObject>(eItem->Entity());
        }
    foreach(ComponentItem *cItem, sel.components)
        if (cItem->Component())
        {
            objects << cItem->Component().get();
            objectPtrs << boost::dynamic_pointer_cast<QObject>(cItem->Component());
        }

    // Shift+click opens existing invoke history item editable in dialog.
    bool openForEditing = contextMenu && contextMenu->shiftDown;
    if (invokedItem->type == InvokeItem::Action)
    {
        if (openForEditing)
        {
            EntityActionDialog *d = new EntityActionDialog(entities, *invokedItem, this);
            connect(d, SIGNAL(finished(int)), this, SLOT(EntityActionDialogFinished(int)));
            d->show();
        }
        else
        {
            foreach(Scene::EntityWeakPtr e, entities)
                e.lock()->Exec(invokedItem->execTypes, invokedItem->name, invokedItem->parameters);
        }
    }
    else if (invokedItem->type == InvokeItem::Function)
    {
        if (openForEditing)
        {
            FunctionDialog *d = new FunctionDialog(objectPtrs, *invokedItem, this);
            connect(d, SIGNAL(finished(int)), SLOT(FunctionDialogFinished(int)));
            d->show();
            d->move(300,300);
        }
        else
        {
            FunctionInvoker invoker;
            foreach(QObject *obj, objects)
            {
                QVariant retVal;
                invoker.Invoke(obj, invokedItem->name, &retVal, invokedItem->parameters);
                LogInfo("Invoked function returned " + retVal.toString().toStdString());
            }
        }
    }
}


void SceneTreeWidget::SaveAssetAs()
{
    Selection sel = GetSelection();
    QString assetName;

    if (fileDialog)
        fileDialog->close();

    if (sel.assets.size() == 1)
    {
        assetName = sel.assets[0]->id.right(sel.assets[0]->id.size() - sel.assets[0]->id.lastIndexOf("://") - 3);

        fileDialog = QtUtils::SaveFileDialogNonModal("",
            tr("Save Asset As"), assetName, 0, this, SLOT(SaveAssetDialogClosed(int)));
    } else
    {
        QtUtils::DirectoryDialogNonModal(tr("Select Directory"), "", 0, this, SLOT(SaveAssetDialogClosed(int)));
    }
}

void SceneTreeWidget::SaveAssetDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);

    if (!dialog || result != QDialog::Accepted || dialog->selectedFiles().isEmpty() || scene.expired())
        return;

    QStringList files = dialog->selectedFiles();
    Selection sel = GetSelection();
    
    bool isDir = QDir(files[0]).exists();

    if ((sel.assets.size() == 1 && isDir) || (sel.assets.size() > 1 && !isDir))
    {
        // should not happen normally, so just log error. No prompt for user.
        LogError("Could not save asset: no such directory.");
        return;
    }

    saved_assets_.clear();
    fetch_references_ = false;
    foreach(AssetItem *aItem, sel.assets)
    {
        AssetTransferPtr transfer = framework->Asset()->RequestAsset(aItem->id);

        // if saving multiple assets, append filename to directory
        QString filename = files[0];
        if (isDir)
        {
            QString assetName = aItem->id.right(aItem->id.size() - aItem->id.lastIndexOf("://") - 3);
            filename += QDir::separator() + assetName;
        }

        filesaves_.insert(transfer, filename);
        connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(AssetLoaded(IAssetTransfer *)));
    }
}

void SceneTreeWidget::AssetLoaded(IAssetTransfer *transfer_)
{
    assert(transfer_);
    if (!transfer_)
        return;

    AssetTransferPtr transfer = transfer_->shared_from_this();
    assert(filesaves_.contains(transfer));
    assert(transfer.get());

    if (!transfer->resourcePtr.get())
    {
        // This means the asset was loaded through the new Asset API, in which case the resourcePtr is null, and the 'asset' member
        // points to the actual loaded asset. For a migration period, we'll need to check both pointers.
        LogWarning("TODO: SceneTreeWidget::AssetLoaded: Received an asset transfer with null resourcePtr. Implement support for this.");
        return;
    }

    QString filename = filesaves_.take(transfer);
    if (!saved_assets_.contains(filename))
    {
        saved_assets_.insert(filename);
        if (!transfer->resourcePtr->Export(filename.toStdString()))
        {
            LogError("Could not save asset to file " + filename.toStdString() + ".");
            QMessageBox box(QMessageBox::Warning, tr("Save asset"), tr("Failed to save asset."), QMessageBox::Ok);
            box.setInformativeText(tr("Please check the selected storage device can be written to."));
            box.exec();
        }

        if (fetch_references_)
        {
            const Foundation::ResourceReferenceVector &refs = transfer->resourcePtr->GetReferences();
            Foundation::ResourceReferenceVector::const_iterator i = refs.begin();
            for ( ; i != refs.end() ; ++i)
            {
                QString id = QString(i->id_.c_str());
                if (!saved_assets_.contains(id))
                {
                    AssetTransferPtr transfer = framework->Asset()->RequestAsset(id, QString(i->type_.c_str()));
                    filesaves_.insert(transfer, filename);
                    connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(AssetLoaded(IAssetTransfer *)));
                }
            }
        }
    }
}
