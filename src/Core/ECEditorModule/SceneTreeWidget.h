/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneTreeWidget.h
    @brief  Tree widget showing the scene structure. */

#pragma once

#include "CoreTypes.h"
#include "SceneFwd.h"
#include "AssetFwd.h"

#include <QTreeWidget>
#include <QPointer>
#include <QMenu>
#include <QShortcut>

class QWidget;
class QFileDialog;

class ECEditorWindow;
class IArgumentType;
class IAssetTransfer;
class EntityItem;
class UndoManager;

struct InvokeItem;
struct SceneTreeWidgetSelection;

/// Context menu for SceneTreeWidget.
/// @cond PRIVATE
class Menu : public QMenu
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param parent Parent widget. */
    explicit Menu(QWidget *parent = 0);

    bool shiftDown; ///< Is shift currently held down.

private:
    /// QWidget override. Checks if shift is pressed.
    void keyPressEvent(QKeyEvent *e);

    /// QWidget override. Checks if shift is released.
    void keyReleaseEvent(QKeyEvent *e);
};
/// @endcond

/// Tree widget showing the scene structure.
class SceneTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework pointer.
        @param parent Parent widget. */
    SceneTreeWidget(Framework *fw, QWidget *parent = 0);

    /// Destructor.
    virtual ~SceneTreeWidget();

    /// Sets new scene to be shown in the tree view.
    /** @param scene Scene which contents we want to modify. */
    void SetScene(const ScenePtr &scene);

    /// Do we show components in the tree widget or not.
    bool showComponents;

    /// @note Available only after the scene is set.
    UndoManager *GetUndoManager() const;

protected:
    /// QAbstractItemView override.
    void contextMenuEvent(QContextMenuEvent *e);

    /// QAbstractItemView override.
    void dragEnterEvent(QDragEnterEvent *e);

    /// QAbstractItemView override.
    void dragMoveEvent(QDragMoveEvent *e);

    /// QAbstractItemView override.
    void dropEvent(QDropEvent *e);

private:
    /// Creates and adds applicable actions to the right-click context menu.
    /** @param menu Context menu. */
    void AddAvailableActions(QMenu *menu);

    /// Creates and adds applicable asset actions to the right-click context menu.
    /** @param menu Context menu. */
    void AddAvailableAssetActions(QMenu *menu);

    /// Creates and adds applicable entity and component actions to the right-click context menu.
    /** @param menu Context menu. */
    void AddAvailableEntityActions(QMenu *menu);

    /// Returns selected items as SceneTreeWidgetSelection struct, which contains both selected entities and components.
    SceneTreeWidgetSelection SelectedItems() const;

    /// Returns currently selected entities as XML string.
    QString SelectionAsXml() const;

    /// Loads invoke history from config file.
    void LoadInvokeHistory();

    /// Saves invoke history to config file.
    void SaveInvokeHistory();

    /// Return most recently used InvokeItem.
    InvokeItem *FindMruItem();

    /// Returns all asset references for the specified entity item.
    QSet<QString> GetAssetRefs(const EntityItem *eItem, bool includeEmptyRefs = true) const;

    Framework *framework; ///< Framework pointer.
    SceneWeakPtr scene; ///< Scene which we are showing the in tree widget currently.
    QList<QPointer<ECEditorWindow> > ecEditors; ///< This EC editors owned by this widget.
    int historyMaxItemCount; ///< Maximum count of invoke history items.
    int numberOfInvokeItemsVisible; ///< Number of visible invoke items in the context-menu.
    QPointer<QFileDialog> fileDialog; ///< Keeps track of the latest opened file save/open dialog.
    QList<InvokeItem> invokeHistory; ///< Keeps track of recently invoked entity actions and functions.
    QPointer<Menu> contextMenu; ///< Context menu.
    UndoManager * undoManager_;
    QShortcut *undoShortcut;
    QShortcut *redoShortcut;

    /// Used when saving multiple assets, can be used to retrieve a matching filename where to save asset data from asset transfer.
    QMap<QString, QString> fileSaves;

    /// Used by 'Export all', a list of assets that have already been saved, so assets are not saved multiple times.
    /** Multiple assets can reference reference another asset, so each asset must be saved only once.
        This must be cleared before starting any assets saving operations. */
    QSet<QString> savedAssets;

    bool fetchReferences; ///< if true, when saving assets, also saves references

private slots:
    /// Opens selected entities in EC editor window. An existing editor window is used if possible.
    void Edit();

    /// Opens selected entities in EC editor window. New editor window is created each time.
    void EditInNew();

    /// Renames selected entity.
    void Rename();

    /// Sets new name for item (entity or component) when it's renamed.
    /** @item The item which was renamed. */
    void OnItemEdited(QTreeWidgetItem *item, int);

    /// Used to restore the entity ID in case the rename action does not make changes to the editing
    /** @editor The editor widget (QLineEdit). */
    void OnCommitData(QWidget * editor);
//    void CloseEditor(QTreeWidgetItem *,QTreeWidgetItem *);

    /// Creates a new entity.
    void NewEntity();

    /// Creates a new component.
    void NewComponent();

    /// Called by Add Component dialog when it's closed.
    /** @param result Result of dialog closure. OK is 1, Cancel is 0. */
    void ComponentDialogFinished(int result);

    /// Deletes an existing entity or component.
    void Delete();

    /// Copies selected entities as XML to clipboard.
    void Copy();

    /// Adds clipboard contents to scene as XML.
    void Paste();

    /// Saves selected entities as XML or binary file.
    void SaveAs();

    /// Saves entire scene as XML or binary file.
    void SaveSceneAs();

    /// Exports the entire scene, entities as either XML or binary, assets in their default (import) format.
    void ExportAll();

    /// Imports OGRE .scene or Tundra .txml / .tbin scene file.
    void Import();

    /// Loads new scene.
    void OpenNewScene();

    /// Opens Entity Action dialog.
    void OpenEntityActionDialog();

    /// Called by Entity Action dialog when it's finished.
    /** @param result Result of finish. Close is 0, Execute and Close is 1, Execute is 2. */
    void EntityActionDialogFinished(int result);

    /// Opens Function dialog.
    void OpenFunctionDialog();

    /// Called by Function dialog when it's finished.
    /** @param result Result of dialog finish. Close is 0, Execute and Close is 1, Execute is 2. */
    void FunctionDialogFinished(int result);

    /// Called by "Save SceneTreeWidgetSelection" save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void SaveSelectionDialogClosed(int result);

    /// Called by "Save Scene" save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void SaveSceneDialogClosed(int result);

    /// Called by "Export all" file dialog when it's closed
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void ExportAllDialogClosed(int result);

    /// Called by open file dialog when it's closed.
    /** @param result Result of dialog closure. Open is 1, Cancel is 0. */
    void OpenFileDialogClosed(int result);

    /// Executes invoke item or opens it for editing.
    void InvokeActionTriggered();

    /// Save selected asset as
    void SaveAssetAs();

    /// Called by SaveAssetAs save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void SaveAssetDialogClosed(int result);

    void AssetLoaded(AssetPtr asset);

    /// If editor is destoyed this method ensures that that object is erased from the list.
    void HandleECEditorDestroyed(QObject *obj);

    /// Creates copy from a replicated entity as new local entity and destroys the original.
    void ConvertEntityToLocal();

    /// Creates copy from a local entity as new replicated entity and destroys the original.
    void ConvertEntityToReplicated();

    /// Sets currently selected entities' temporary property.
    /** @param bool temporary Do we want to set or unset the temporary property. */
    void SetAsTemporary(bool temporary);

    /// Groups selected entities, or adds entities to an existing group.
    void GroupEntities();

    /// Ungroups entity or entities. If 'Ungroup' clicked onto a group, it will ungroup all entities within said group. Otherwise ungroups selected entity / entities
    void UngroupEntities();
};
