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

#include <QDomDocument>
#include <QDomElement>

#include "MemoryLeakCheck.h"

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

    // Create context menu actions, connect them to slots and add to menu
    // paste and new actions are available always
    QAction *pasteAction = new QAction(tr("Paste"), menu);
    QAction *newAction = new QAction(tr("New entity"), menu);

    connect(newAction, SIGNAL(triggered()), SLOT(New()));
    connect(pasteAction, SIGNAL(triggered()), SLOT(Paste()));

    // Edit, edit in new, delete, rename and copy actions are available only if we have valid index active
    QAction *editAction = 0, *editInNewAction = 0, *deleteAction = 0, *renameAction = 0,
        *copyAction = 0, *saveAsXmlAction = 0, *saveAsBinaryAction = 0;
    QModelIndex index = selectionModel()->currentIndex();
    if (index.isValid())
    {
        editAction = new QAction(tr("Edit"), menu);
        editInNewAction = new QAction(tr("Edit in new window"), menu);
        deleteAction = new QAction(tr("Delete"), menu);
        //renameAction = new QAction(tr("Rename"), menu);
        copyAction = new QAction(tr("Copy"), menu);
        saveAsXmlAction = new QAction(tr("Save as XML..."), menu);
        saveAsBinaryAction = new QAction(tr("Save as binary..."), menu);

        connect(editAction, SIGNAL(triggered()), SLOT(Edit()));
        connect(editInNewAction, SIGNAL(triggered()), SLOT(EditInNew()));
        connect(deleteAction, SIGNAL(triggered()), SLOT(Delete()));
    //    connect(renameAction, SIGNAL(triggered()), SLOT(Rename()));
        connect(copyAction, SIGNAL(triggered()), SLOT(Copy()));
        connect(saveAsXmlAction, SIGNAL(triggered()), SLOT(SaveAsXml()));
        connect(saveAsBinaryAction, SIGNAL(triggered()), SLOT(SaveAsBinary()));
    }

//    menu->addAction(renameAction);
    if (index.isValid())
    {
        menu->addAction(editAction);
        menu->addAction(editInNewAction);
    }

    menu->addAction(newAction);
    if (index.isValid())
    {
        menu->addAction(deleteAction);
        menu->addAction(copyAction);
    }

    menu->addAction(pasteAction);

    if (index.isValid())
        menu->addAction(saveAsXmlAction);

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

QString SceneTreeWidget::GetSelectionAsXml() const
{
    const Scene::ScenePtr scene = framework->GetDefaultWorldScene();
    assert(scene.get());
    if (!scene)
        return QString();

    const QItemSelection &selection = selectionModel()->selection();
    if (selection.isEmpty())
        return QString();

    // Gather entity ID's.
    QList<entity_id_t> ids;
    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (!index.isValid())
            continue;
        ids << static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()))->id;
    }

    if (ids.size() == 0)
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
    const QItemSelection &selection = selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QList<entity_id_t> ids;
    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (!index.isValid())
            continue;
        ids << static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()))->id;
    }

    if (ids.size() == 0)
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
    const QItemSelection &selection = selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QList<entity_id_t> ids;
    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (!index.isValid())
            continue;
        ids << static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()))->id;
    }

    if (ids.size() == 0)
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
        std::cout << "SceneTreeWidget::New(): invalid entity type:" << type.toStdString() << std::endl;
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

    const QItemSelection &selection = selectionModel()->selection();
    if (selection.isEmpty())
        return;

    // Gather entity ID's.
    QList<entity_id_t> ids;
    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (!index.isValid())
            continue;
        ids << static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()))->id;
    }

    if (ids.size() == 0)
        return;

    // Remove entities.
    foreach(entity_id_t id, ids)
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
        std::cout << "Parsing scene XML from clipboard failed!" << std::endl;
        return;
    }

    scene->CreateContentFromXml(scene_doc, false, AttributeChange::Replicate);
}

void SceneTreeWidget::SaveAsXml()
{
    QFileDialog * dialog = Foundation::QtUtils::SaveFileDialogNonModal(
        "XML (*.xml)"/*;;Naali Binary Format (*.nbf)"*/, tr("Save"), "", this, this, SLOT(SaveFileDialogClosed(int)));
    dialog->setDefaultSuffix("xml");
}

void SceneTreeWidget::SaveAsBinary()
{
    QFileDialog * dialog = Foundation::QtUtils::SaveFileDialogNonModal(
        "Naali Binary Format (*.nbf);;XML (*.xml)", tr("Save"), "", this, this, SLOT(SaveFileDialogClosed(int)));
    dialog->setDefaultSuffix("nbf");
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

    if (files[0].toLower().indexOf(".xml") != -1)
    {
        QString sceneXml = GetSelectionAsXml();
        if (sceneXml.isEmpty())
            return;

        QFile file(files[0]);
        if (!file.open(QIODevice::WriteOnly))
            return;

        file.write(sceneXml.toStdString().c_str());
        file.close();
    }

/*
    else if (files[0].toLower().indexOf(".nbf") != -1)
    {
    }
*/
}
