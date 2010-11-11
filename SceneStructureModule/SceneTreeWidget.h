/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.h
 *  @brief  Tree widget showing the scene structure.
 */

#ifndef incl_SceneStructureModule_SceneTreeWidget_h
#define incl_SceneStructureModule_SceneTreeWidget_h

#include "CoreTypes.h"
#include "ForwardDefines.h"
#include "EntityAction.h"

#include <QTreeWidget>
#include <QPointer>
#include <QMenu>

namespace ECEditor
{
    class ECEditorWindow;
}

struct InvokeItem;
class IArgumentType;

class QWidget;
class QFileDialog;

/// Tree widget item representing an entity.
class EntityItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param entity Entity pointer.
    */
    explicit EntityItem(const Scene::EntityPtr &entity);

    /// Returns pointer to the entity this item represents.
    Scene::EntityPtr Entity() const;

    /// Return Entity ID of the entity associated with this tree widget item.
    entity_id_t Id() const;

private:
    entity_id_t id; ///< Entity ID associated with this tree widget item.
    Scene::EntityWeakPtr ptr; ///< Weak pointer to the component this item represents.
};

/// Tree widget item representing a component.
class ComponentItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param comp Component pointer.
        @param parent Parent entity item.
    */
    ComponentItem(const ComponentPtr &comp, EntityItem *parent);

    /// Returns pointer to the entity this item represents.
    ComponentPtr Component() const;

    /// Returns the parent entity item.
    EntityItem *Parent() const;

    QString typeName; ///< Type name.
    QString name; ///< Name, if applicable.

private:
    ComponentWeakPtr ptr; ///< Weak pointer to the component this item represents.
    EntityItem *parentItem; ///< Parent entity item.
};

/// Tree widget item representing an asset reference.
class AssetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param name Name of the asset reference attribute.
        @param ref Asset reference URI.
        @param parent Parent item.
    */
    AssetItem(const QString &name, const QString &ref, QTreeWidgetItem *parent = 0);

    QString name; ///< Name of the attribute.
    QString id; ///< ID.
    QString type; ///< Type.
};

/// Represents selection of selected scene tree widget items.
struct Selection
{
    /// Returns true if no entity or component items selected.
    bool IsEmpty() const;

    /// Returns true if selection contains entities;
    bool HasEntities() const;

    /// Returns true if selected contains components.
    bool HasComponents() const;

    /// Returns list containing unique entity ID's of both selected entities and parent entities of selected components
    QList<entity_id_t> EntityIds() const;

    QList<EntityItem *> entities; ///< List of selected entities.
    QList<ComponentItem *> components; ///< List of selected components.
};

/// Context menu for SceneTreeWidget.
class Menu : public QMenu
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param parent Parent widget.
    */
    explicit Menu(QWidget *parent = 0);

    /// Is shift currently held down.
    bool shiftDown;
private:
    /// QWidget override. Checks if shift is pressed.
    void keyPressEvent(QKeyEvent *e);

    /// QWidget override. Checks if shift is released.
    void keyReleaseEvent(QKeyEvent *e);
};

/// Tree widget showing the scene structure.
class SceneTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework pointer.
        @param parent Parent widget.
    */
    SceneTreeWidget(Foundation::Framework *fw, QWidget *parent = 0);

    /// Destructor.
    virtual ~SceneTreeWidget();

    /// Sets new scene to be shown in the tree view.
    /** @param scene Scene which contents we want to modify.
    */
    void SetScene(const Scene::ScenePtr &scene);

    /// Do we show components in the tree widget or not.
    bool showComponents;

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
    /** @param [out] menu Context menu.
    */
    void AddAvailableActions(QMenu *menu);

    /// Returns selected items as Selection struct, which contains both selected entities and components.
    Selection GetSelection() const;

    /// Returns currently selected entities as XML string.
    QString GetSelectionAsXml() const;

        /// Loads invoke history from config file.
    void LoadInvokeHistory();

    /// Saves incoke history to config file.
    void SaveInvokeHistory();

    /// Return most recently used InvokeItem.
    InvokeItem *FindMruItem() const;

    /// Framework pointer.
    Foundation::Framework *framework;

    /// Scene which we are showing the in tree widget currently.
    Scene::SceneWeakPtr scene;

    /// This widget's "own" EC editor.
    QPointer<ECEditor::ECEditorWindow> ecEditor;

    /// Maximum count of invoke history items.
    int historyMaxItemCount;

    /// Number of visible invoke items in the context-menu.
    int numberOfInvokeItemsVisible;

    /// Keeps track of the latest opened file save/open dialog, so that we won't multiple open at the same time.
    QPointer<QFileDialog> fileDialog;

    /// Keeps track of recently invoked entity actions and functions.
    QList<InvokeItem> invokeHistory;

    /// Context menu.
    QPointer<Menu> contextMenu;

private slots:
    /// Opens selected entities in EC editor window. An existing editor window is used if possible.
    void Edit();

    /// Opens selected entities in EC editor window. New editor window is created each time.
    void EditInNew();

    /// Renames selected entity.
    void Rename();

    /// Sets new name for item (entity or component) when it's renamed.
    /** @item The item which was renamed.
    */
    void OnItemEdited(QTreeWidgetItem *item, int);

    void CloseEditor(QTreeWidgetItem *,QTreeWidgetItem *);

    /// Creates a new entity.
    void NewEntity();

    /// Creates a new component.
    void NewComponent();

    /// Called by Add Component dialog when it's closed.
    /** @param result Result of dialog closure. OK is 1, Cancel is 0.
    */
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

    /// Imports OGRE or Naali scene file.
    void Import();

    /// Loads new scene.
    void OpenNewScene();

    /// Opens Entity Action dialog.
    void OpenEntityActionDialog();

    /// Called by Entity Action dialog when it's finished.
    /** @param result Result of finish. Close is 0, Execute and Close is 1, Execute is 2.
    */
    void EntityActionDialogFinished(int result);

    /// Opens Function dialog.
    void OpenFunctionDialog();

    /// Called by Function dialog when it's finished.
    /** @param result Result of dialog finish. Close is 0, Execute and Close is 1, Execute is 2.
    */
    void FunctionDialogFinished(int result);

    /// Called by "Save Selection" save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0.
    */
    void SaveSelectionDialogClosed(int result);

    /// Called by "Save Scene" save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0.
    */
    void SaveSceneDialogClosed(int result);

    /// Called by open file dialog when it's closed.
    /** @param result Result of dialog closure. Open is 1, Cancel is 0.
    */
    void OpenFileDialogClosed(int result);

    /// Executes invoke item or opens it for editing.
    void InvokeActionTriggered();
};

#endif
