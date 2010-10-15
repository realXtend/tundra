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

#include "ECEditorWindow.h"
#include "UiServiceInterface.h"
#include "SceneManager.h"
#include "QtUtils.h"
#include "LoggingFunctions.h"
#include "SceneImporter.h"
#include "ComponentManager.h"
#include "ModuleManager.h"
#include "TundraEvents.h"
#include "EventManager.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneTreeView");

#include <QDomDocument>
#include <QDomElement>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

const QString cOgreSceneFileFilter(QApplication::translate("SceneTreeWidget", "OGRE scene (*.scene)"));
const QString cOgreMeshFileFilter(QApplication::translate("SceneTreeWidget", "OGRE mesh (*.mesh)"));
const QString cNaaliXmlFileFilter(QApplication::translate("SceneTreeWidget", "Naali scene XML(*.xml)"));
const QString cNaaliBinaryFileFilter(QApplication::translate("SceneTreeWidget", "Naali Binary Format (*.nbf)"));
const QString cAllSupportedTypesFileFilter(QApplication::translate("SceneTreeWidget", "All supported types (*.scene *.mesh *.xml *.nbf)"));

SceneTreeWidget::SceneTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    showComponents(false)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers/*EditKeyPressed*/);
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
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C), this);
    QShortcut *pasteShortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V), this);

    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(Delete()));
    connect(copyShortcut, SIGNAL(activated()), this, SLOT(Copy()));
    connect(pasteShortcut, SIGNAL(activated()), this, SLOT(Paste()));
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

    // Create context menu
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Create context menu actions
    // "New entity...", "Import..." and "Open new scene..." actions are available always
    QAction *newAction = new QAction(tr("New entity..."), menu);
    QAction *importAction = new QAction(tr("Import..."), menu);
    QAction *openNewSceneAction = new QAction(tr("Open new scene..."), menu);

    connect(newAction, SIGNAL(triggered()), SLOT(New()));
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

    // "Edit", "Edit in new", "Delete",and "Copy" actions are available only if we have selection.
    QAction *editAction = 0, *editInNewAction = 0, *deleteAction = 0, *renameAction = 0,
        *copyAction = 0, *saveAsAction = 0;

    bool hasSelection = !selectionModel()->selection().isEmpty();
    if (hasSelection)
    {
        editAction = new QAction(tr("Edit"), menu);
        editInNewAction = new QAction(tr("Edit in new window"), menu);
        deleteAction = new QAction(tr("Delete"), menu);
        copyAction = new QAction(tr("Copy"), menu);
        saveAsAction = new QAction(tr("Save as..."), menu);

        connect(editAction, SIGNAL(triggered()), SLOT(Edit()));
        connect(editInNewAction, SIGNAL(triggered()), SLOT(EditInNew()));
        connect(deleteAction, SIGNAL(triggered()), SLOT(Delete()));
        connect(copyAction, SIGNAL(triggered()), SLOT(Copy()));
        connect(saveAsAction, SIGNAL(triggered()), SLOT(SaveAs()));
    }

    // "Rename" action is possible only if have one entity selected.
/*
    bool renamePossible = (selectionModel()->selection().size() == 1)
    if (renamePossible)
    {
        renameAction = new QAction(tr("Rename"), menu);
        connect(renameAction, SIGNAL(triggered()), SLOT(Rename()));
    }

    if (renamePossible)
        menu->addAction(renameAction);
*/
    if (hasSelection)
    {
        menu->addAction(editAction);
        menu->addAction(editInNewAction);
    }

    menu->addAction(newAction);

    if (hasSelection)
    {
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

    // Show menu.
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
        foreach(QUrl url, data->urls())
        {
            QString filename = url.path();
#ifdef _WINDOWS
            // We have '/' as the first char on windows and the filename
            // is not identified as a file properly. But on other platforms the '/' is valid/required.
            filename = filename.mid(1);
#endif
            framework->GetModule<SceneStructureModule>()->InstantiateContent(filename, Vector3df(), false);
        }

        e->acceptProposedAction();
    }
    else
        QTreeWidget::dropEvent(e);
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
    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return QString();

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
            Scene::EntityPtr entity = scene->GetEntity(eItem->id);
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
            EntityItem *eItem = dynamic_cast<EntityItem *>(cItem->parent());
            if (eItem)
            {
                Scene::EntityPtr entity = scene->GetEntity(eItem->id);
                if (entity)
                {
                    ComponentPtr component = entity->GetComponent(cItem->typeName, cItem->name);
                    if (component->IsSerializable())
                        component->SerializeTo(scene_doc, scene_elem);
                }
            }
        }

        scene_doc.appendChild(scene_elem);
    }

    return scene_doc.toString();
}

void SceneTreeWidget::Edit()
{
    Selection selection = GetSelection();
    if (selection.IsEmpty())
        return;

    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    // If we have existing editor instance, use it.
    if (ecEditor)
    {
        foreach(entity_id_t id, selection.EntityIds())
            ecEditor->AddEntity(id);
        ecEditor->SetSelectedEntities(selection.EntityIds().toList());
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
    ecEditor->SetSelectedEntities(selection.EntityIds().toList());

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
    editor->SetSelectedEntities(selection.EntityIds().toList());

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
}

void SceneTreeWidget::New()
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

void SceneTreeWidget::Delete()
{
    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    // Remove entities.
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
            Scene::EntityPtr entity = scene->GetEntity(eItem->id);
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
        cOgreSceneFileFilter + ";;" + cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter + ";;" + cOgreMeshFileFilter,
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

void SceneTreeWidget::SaveSelectionDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != 1)
        return;

    QStringList files = dialog->selectedFiles();
    if (files.size() != 1)
        return;

    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
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

            foreach(EntityItem *e, sel.entities)
            {
                Scene::EntityPtr entity = scene->GetEntity(e->id);
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

    if (result != 1)
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

    if (result != 1)
        return;

    const Scene::ScenePtr &scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    foreach(QString filename, dialog->selectedFiles())
    {
        bool clearScene = false;
        ///\todo This is awful hack, find better way
        if (dialog->windowTitle() == tr("Open New Scene"))
            clearScene = true;

        framework->GetModule<SceneStructureModule>()->InstantiateContent(filename, Vector3df(), clearScene);
    }
}

