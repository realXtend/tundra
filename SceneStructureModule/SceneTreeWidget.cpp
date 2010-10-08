/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.cpp
 *  @brief  Tree widget showing the scene structure.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SceneTreeWidget.h"

#include "ECEditorWindow.h"
#include "UiServiceInterface.h"
#include "SceneManager.h"
#include "QtUtils.h"
#include "LoggingFunctions.h"
#include "SceneImporter.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneTreeView");

#include <QDomDocument>
#include <QDomElement>

#include <kNet/DataDeserializer.h>
#include <kNet/DataSerializer.h>

#include "MemoryLeakCheck.h"

const QString cOgreSceneFileFilter("OGRE scene (*.scene)");
const QString cNaaliXmlFileFilter("Naali scene XML(*.xml)");
const QString cNaaliBinaryFileFilter("Naali Binary Format (*.nbf)");

SceneTreeWidget::SceneTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers/*EditKeyPressed*/);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    //setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setHeaderHidden(true);
/*
    setStyleSheet(
        "QTreeView::branch:has-siblings:!adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchVLine.png) 0;"
        "}"
        "QTreeView::branch:has-siblings:adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchMore.png) 0;"
        "}"
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchEnd.png) 0;"
        "}"
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings"
        "{"
            "border-image: none;"
            "image: url(:/images/iconBranchClosed.png);"
        "}"
        "QTreeView::branch:open:has-children:!has-siblings,"
        "QTreeView::branch:open:has-children:has-siblings"
        "{"
            "border-image: none;"
            "image: url(:/images/iconBranchOpen.png);"
        "}");
*/
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
    // paste, new entity, import OGRE scene, open new scene and import content from scene
    // actions are available always
    QAction *pasteAction = new QAction(tr("Paste"), menu);
    QAction *newAction = new QAction(tr("New entity..."), menu);
    QAction *importAction = new QAction(tr("Import..."), menu);
    QAction *openNewSceneAction = new QAction(tr("Load new scene..."), menu);

    connect(newAction, SIGNAL(triggered()), SLOT(New()));
    connect(pasteAction, SIGNAL(triggered()), SLOT(Paste()));
    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    connect(openNewSceneAction, SIGNAL(triggered()), SLOT(OpenNewScene()));

    // Edit, edit in new, delete, rename and copy actions are available only if we have valid index active
    QAction *editAction = 0, *editInNewAction = 0, *deleteAction = 0, *renameAction = 0,
        *copyAction = 0, *saveAsAction = 0;

    bool hasSelection = !selectionModel()->selection().isEmpty();
    if (hasSelection)
    {
        editAction = new QAction(tr("Edit"), menu);
        editInNewAction = new QAction(tr("Edit in new window"), menu);
        deleteAction = new QAction(tr("Delete"), menu);
        //renameAction = new QAction(tr("Rename"), menu);
        copyAction = new QAction(tr("Copy"), menu);
        saveAsAction = new QAction(tr("Save as..."), menu);

        connect(editAction, SIGNAL(triggered()), SLOT(Edit()));
        connect(editInNewAction, SIGNAL(triggered()), SLOT(EditInNew()));
        connect(deleteAction, SIGNAL(triggered()), SLOT(Delete()));
    //    connect(renameAction, SIGNAL(triggered()), SLOT(Rename()));
        connect(copyAction, SIGNAL(triggered()), SLOT(Copy()));
        connect(saveAsAction, SIGNAL(triggered()), SLOT(SaveAs()));
    }

//    menu->addAction(renameAction);
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

    menu->addAction(pasteAction);

    menu->addSeparator();

    if (hasSelection)
        menu->addAction(saveAsAction);

    menu->addAction(importAction);
    menu->addAction(openNewSceneAction);

    // Show menu.
    menu->popup(e->globalPos());
}

void SceneTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
/*
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
*/
}

void SceneTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
/*
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            InventoryItemModel *itemModel = checked_static_cast<InventoryItemModel *>(model());
            assert(itemModel);

            AbstractInventoryItem *draggedItem = itemModel->GetItem(selectionModel()->currentIndex());
            assert(draggedItem);
            QModelIndex destIndex = indexAt(event->pos());

            if (!destIndex.isValid())
            {
                event->ignore();
                return;
            }

            AbstractInventoryItem *destItem = 0;

            if (itemModel->GetItem(destIndex)->GetItemType() == AbstractInventoryItem::Type_Asset)
                destItem = itemModel->GetItem(destIndex)->GetParent();
            else
                destItem = itemModel->GetItem(destIndex);

            if (!destItem || destItem->IsLibraryItem() || (draggedItem->GetParent() == destItem))
            {
                event->ignore();
                return;
            }

            if (draggedItem ->IsLibraryItem())
                event->setDropAction(Qt::CopyAction);
            else
                event->setDropAction(Qt::MoveAction);

            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
*/
}

void SceneTreeWidget::dropEvent(QDropEvent *e)
{
/*
    const QMimeData *data = event->mimeData();
    if (data->hasUrls())
    {
        InventoryItemModel *itemModel = dynamic_cast<InventoryItemModel *>(model());
        if (!itemModel)
        {
            event->ignore();
            return;
        }

        AbstractInventoryDataModel *m = itemModel->GetInventory();
        if (!m)
        {
            event->ignore();
            return;
        }

        QStringList filenames, itemnames;
        QListIterator<QUrl> it(data->urls());
        while(it.hasNext())
            filenames << it.next().path();

        if (!filenames.isEmpty())
            m->UploadFiles(filenames, itemnames, 0);

        event->acceptProposedAction();

        itemModel->CheckTreeForDirtys();
    }
    else
        QTreeView::dropEvent(event);
*/
}

QList<entity_id_t> SceneTreeWidget::GetSelectedEntities() const
{
    QList<entity_id_t> ids;
    const QItemSelection &selection = selectionModel()->selection();
    if (selection.isEmpty())
        return ids;

    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (!index.isValid())
            continue;
        ids << static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()))->id;
    }

    return ids;
}

QString SceneTreeWidget::GetSelectionAsXml() const
{
    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return QString();

    QList<entity_id_t> ids = GetSelectedEntities();
    if (ids.empty())
        return QString();

    // Create root Scene element always for consistency, even if we only have one entity
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");

    foreach(entity_id_t id, ids)
    {
        Scene::EntityPtr entity = scene->GetEntity(id);
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

    return scene_doc.toString();
}

void SceneTreeWidget::Edit()
{
    QList<entity_id_t> ids = GetSelectedEntities();
    if (ids.empty())
        return;

    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    // If we have existing editor instance, use it.
    if (ecEditor)
    {
        foreach(entity_id_t id, ids)
            ecEditor->AddEntity(id);
        ecEditor->SetSelectedEntities(ids);
        ui->BringWidgetToFront(ecEditor);
        return;
    }

    // Create new instance
    ecEditor = new ECEditor::ECEditorWindow(framework);
    ecEditor->setAttribute(Qt::WA_DeleteOnClose);
    ecEditor->move(mapToGlobal(pos()) + QPoint(50, 50));
    ecEditor->hide();
    foreach(entity_id_t id, ids)
        ecEditor->AddEntity(id);
    ecEditor->SetSelectedEntities(ids);

    ui->AddWidgetToScene(ecEditor);
    ui->ShowWidget(ecEditor);
    ui->BringWidgetToFront(ecEditor);
}

void SceneTreeWidget::EditInNew()
{
    QList<entity_id_t> ids = GetSelectedEntities();
    if (ids.empty())
        return;

    // Create new instance every time.
    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    ECEditor::ECEditorWindow *editor = new ECEditor::ECEditorWindow(framework);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->move(mapToGlobal(pos()) + QPoint(50, 50));
    editor->hide();
    foreach(entity_id_t id, ids)
        editor->AddEntity(id);
    editor->SetSelectedEntities(ids);

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
    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    entity_id_t id;
    AttributeChange::Type changeType;

    // Show a dialog so that user can choose if he wants to create local or synchronized entity.
    QStringList types(QStringList() << "Local" << "Synchronized");
    bool ok;
    QString type = QInputDialog::getItem(this, tr("Choose Entity Type"), tr("Type:"), types, 0, false, &ok);
    if (!ok || type.isEmpty())
        return;

    if (type == tr("Local"))
    {
        id =scene->GetNextFreeIdLocal();
        changeType = AttributeChange::LocalOnly;
    }
    else if(type == tr("Synchronized"))
    {
        id = scene->GetNextFreeId();
        changeType = AttributeChange::Replicate;
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
    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    // Remove entities.
    foreach(entity_id_t id, GetSelectedEntities())
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
    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;
/*
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;
*/
    QDomDocument scene_doc("Scene");
    if (!scene_doc.setContent(QApplication::clipboard()->text()))
    {
        LogError("Parsing scene XML from clipboard failed!");
        return;
    }

    scene->CreateContentFromXml(scene_doc, false, AttributeChange::Replicate);
}

void SceneTreeWidget::SaveAs()
{
    QFileDialog * dialog = Foundation::QtUtils::SaveFileDialogNonModal(
        cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter, tr("Save"), "", 0, this, SLOT(SaveFileDialogClosed(int)));
}

void SceneTreeWidget::Import()
{
    QFileDialog * dialog = Foundation::QtUtils::OpenFileDialogNonModal(
        cOgreSceneFileFilter + ";;" + cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter, // + Mesh
        tr("Import"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::OpenNewScene()
{
    QFileDialog * dialog = Foundation::QtUtils::OpenFileDialogNonModal(
        cOgreSceneFileFilter + ";;" + cNaaliXmlFileFilter + ";;" + cNaaliBinaryFileFilter,
        tr("Open New Scene"), "", 0, this, SLOT(OpenFileDialogClosed(int)));
}

void SceneTreeWidget::SaveFileDialogClosed(int result)
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

    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
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
        QList<entity_id_t> ids = GetSelectedEntities();
        if (!ids.empty())
        {
            // Assume 4MB max for now
            bytes.resize(4 * 1024 * 1024);
            kNet::DataSerializer dest(bytes.data(), bytes.size());

            dest.Add<u32>(ids.size());

            foreach(entity_id_t id, ids)
            {
                Scene::EntityPtr entity = scene->GetEntity(id);
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

void SceneTreeWidget::OpenFileDialogClosed(int result)
{
    QFileDialog *dialog = dynamic_cast<QFileDialog *>(sender());
    assert(dialog);
    if (!dialog)
        return;

    if (result != 1)
        return;

    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return;

    foreach(QString filename, dialog->selectedFiles())
        if (filename.toLower().indexOf(".scene") != -1)
        {
            boost::filesystem::path path(filename.toStdString());
            std::string dirname = path.branch_path().string();

            TundraLogic::SceneImporter importer(framework);
            bool clearScene = false;
            ///\todo This is awful hack, find better way
            if (dialog->windowTitle() == tr("Open New Scene"))
                clearScene = true;

            ///\todo Take into account asset sources.
            importer.Import(scene, filename.toStdString(), dirname, "./data/assets", AttributeChange::Default, clearScene, true, true);
        }
        else if (filename.toLower().indexOf(".xml") != -1)
        {
            ///\todo This is awful hack, find better way
            bool clearScene = false;
            if (dialog->windowTitle() == tr("Open New Scene"))
                clearScene = true;
            scene->LoadSceneXML(filename.toStdString(), clearScene, AttributeChange::Replicate);
        }
        else if (filename.toLower().indexOf(".nbf") != -1)
        {
            scene->CreateContentFromBinary(filename, true, AttributeChange::Replicate);
        }
        else
        {
            LogError("Unsupported file extension: " + filename.toStdString());
        }
}
