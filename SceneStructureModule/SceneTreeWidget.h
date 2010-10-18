/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.h
 *  @brief  Tree widget showing the scene structure.
 */

#ifndef incl_InventoryModule_InventoryTreeView_h
#define incl_InventoryModule_InventoryTreeView_h

#include "CoreTypes.h"

#include <QTreeWidget>
#include <QPointer>

namespace Foundation
{
    class Framework;
}

namespace ECEditor
{
    class ECEditorWindow;
}

class QWidget;
class QFileDialog;

/// Tree widget item representing entity.
class EntityItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param entityID Entity ID.
    */
    EntityItem(entity_id_t entityId) : id(entityId) {}

    /// Entity ID associated with this tree widget item.
    entity_id_t id;
};

/// Tree widget item representing component.
class ComponentItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param tn Type name.
        @param n Name, if applicable.
        @param parent Parent item.
    */
    explicit ComponentItem(const QString &tn, const QString &n, QTreeWidgetItem *parent = 0) :
        QTreeWidgetItem(parent), typeName(tn), name(n) {}

    /// Type name.
    QString typeName;

    ///  Name, if applicable.
    QString name;
};

/// Tree widget item representing asset reference.
class AssetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param i ID.
        @param t Type.
        @param parent Parent item.
    */
    explicit AssetItem(const QString &i, const QString &t, QTreeWidgetItem *parent = 0) :
        QTreeWidgetItem(parent), id(i), type(t) {}

    /// ID.
    QString id;

    /// Type.
    QString type;
};

/// Represents selection of selected scene tree widget items.
struct Selection
{
    /// Returns true if no entity or component items selected.
    bool IsEmpty() const { return entities.size() == 0 && components.size() == 0; }

    /// Returns true if selection contains entities;
    bool HasEntities() const { return entities.size() > 0; }

    /// Returns true if selected contains components.
    bool HasComponents() const { return components.size() > 0; }

    /// Returns set containing entity ID's of both selected entities and parent entities of selected components
    QSet<entity_id_t> EntityIds() const
    {
        QSet<entity_id_t> ids;
        foreach(EntityItem *e, entities)
            ids.insert(e->id);
        foreach(ComponentItem *c, components)
        {
            EntityItem *e = dynamic_cast<EntityItem *>(c->parent());
            if (e)
                ids.insert(e->id);
        }

        return ids;
    }

    /// List of selected entities.
    QList<EntityItem *> entities;

    /// List of selected components.
    QList<ComponentItem *> components;
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
    /// Framework pointer.
    Foundation::Framework *framework;

    /// This widget's "own" EC editor.
    QPointer<ECEditor::ECEditorWindow> ecEditor;

    /// Returns selected items as Selection struct, which contains both selected entities and components.
    Selection GetSelection() const;

    /// Returns currently selected entities as XML string.
    QString GetSelectionAsXml() const;

    /// Keeps track of the latest opened file save/open dialog, so that we won't multiple open at the same time.
    QPointer<QFileDialog> fileDialog;

private slots:
    /// Opens selected entities in EC editor window. An exisiting editor window is used if possible.
    void Edit();

    /// Opens selected entities in EC editor window. New editor window is created each time.
    void EditInNew();

    /// Renames selected entity.
    void Rename();

    ///
    /** @item The item which was edited.
    */
    void OnItemEdited(QTreeWidgetItem *item);

    /// Creates new entity.
    void New();

    /// Deletes an existing entity.
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
};

#endif
