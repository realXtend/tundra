/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "SceneStructureWindow.h"
#include "SceneTreeWidget.h"

#include "Framework.h"
#include "SceneManager.h"
#include "ECEditorWindow.h"
#include "UiServiceInterface.h"

using namespace Scene;

SceneStructureWindow::SceneStructureWindow(Foundation::Framework *fw) : framework(fw)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setWindowTitle(tr("Scene Structure"));
    resize(200,300);

    treeWidget = new SceneTreeWidget(this);
    layout->addWidget(treeWidget);

    connect(treeWidget, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(EditEntity(const QModelIndex &)));
}

SceneStructureWindow::~SceneStructureWindow()
{
    Clear();
}

void SceneStructureWindow::SetScene(const Scene::ScenePtr &s)
{
    scene = s;
    SceneManager *scenePtr = scene.lock().get();
    assert(scenePtr);
    connect(scenePtr, SIGNAL(EntityCreated(Scene::Entity *, AttributeChange::Type)), SLOT(AddEntity(Scene::Entity *)));
    connect(scenePtr, SIGNAL(EntityRemoved(Scene::Entity *, AttributeChange::Type)), SLOT(RemoveEntity(Scene::Entity *)));
    Populate();
}

void SceneStructureWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange)
        setWindowTitle(tr("Scene Structure"));
    else
        QWidget::changeEvent(e);
}

void SceneStructureWindow::Populate()
{
    ScenePtr s = scene.lock();
    if (!s)
    {
        // warning print
        return;
    }

    SceneManager::iterator it = s->begin();
    while(it != s->end())
    {
        const EntityPtr &e = *it;
        SceneTreeWidgetItem *item = new SceneTreeWidgetItem(e->GetId());
        item->setText(0, QString("%1 %2").arg(e->GetId()).arg(e->GetName()));
        treeWidget->addTopLevelItem(item);
        ++it;
    }
}

void SceneStructureWindow::Clear()
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        SAFE_DELETE(item);
    }
}

void SceneStructureWindow::AddEntity(Scene::Entity* entity)
{
    SceneTreeWidgetItem *item = new SceneTreeWidgetItem(entity->GetId());
    item->setText(0, QString("%1 %2").arg(entity->GetId()).arg(entity->GetName()));
    treeWidget->addTopLevelItem(item);
}

void SceneStructureWindow::RemoveEntity(Scene::Entity* entity)
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        SceneTreeWidgetItem *item = static_cast<SceneTreeWidgetItem *>(treeWidget->topLevelItem(i));
        if (item && (item->id == entity->GetId()))
        {
            SAFE_DELETE(item);
            break;
        }
    }
}

void SceneStructureWindow::EditEntity(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (scene.expired())
        return;

    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();

    SceneTreeWidgetItem *item = static_cast<SceneTreeWidgetItem *>(treeWidget->topLevelItem(index.row()));
    Scene::EntityPtr entity = scene.lock()->GetEntity(item->id);
    assert(entity.get());

    ECEditor::ECEditorWindow *editor = new ECEditor::ECEditorWindow(framework);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->move(this->pos().x() + 100, this->pos().y() + 100);
    editor->hide();
    editor->AddEntity(item->id);
    editor->RefreshPropertyBrowser();

    ui->AddWidgetToScene(editor);
    ui->ShowWidget(editor);
    ui->BringWidgetToFront(editor);
}

/*
void InventoryWindow::UpdateActions()
{
    InventoryAction *actionNewFolder = 0, *actionDelete = 0, *actionRename = 0,
        *actionCopyAssetReference = 0, *actionOpen = 0, *actionProperties = 0;

    actionNewFolder = findChild<InventoryAction *>("NewFolder");
    actionDelete = findChild<InventoryAction *>("Delete");
    actionRename = findChild<InventoryAction *>("Rename");
    actionCopyAssetReference = findChild<InventoryAction *>("CopyAssetReference");
    actionOpen = findChild<InventoryAction *>("Open");
    actionProperties = findChild<InventoryAction *>("Properties");

    QModelIndex index = treeView_->selectionModel()->currentIndex();

    bool canAddFolder = !inventoryItemModel_->IsLibraryItem(index);
    if (actionNewFolder)
        actionNewFolder->setEnabled(canAddFolder);

    bool editable = treeView_->model()->flags(index) & Qt::ItemIsEditable;
    if (actionDelete)
        actionDelete->setEnabled(editable);
    if (actionRename)
        actionRename->setEnabled(editable);

    bool is_asset = inventoryItemModel_->GetItemType(index) == AbstractInventoryItem::Type_Asset;
    if (actionCopyAssetReference)
        actionCopyAssetReference->setEnabled(is_asset);
    if (actionOpen)
        actionOpen->setEnabled(is_asset);
    if (actionProperties)
        actionProperties->setEnabled(is_asset);

    bool hasCurrent = treeView_->selectionModel()->currentIndex().isValid();

    if (hasCurrent)
        treeView_->closePersistentEditor(treeView_->selectionModel()->currentIndex());
}*/
