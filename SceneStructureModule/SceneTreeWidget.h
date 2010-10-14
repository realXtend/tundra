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

    /// Framework pointer.
    QPointer<ECEditor::ECEditorWindow> ecEditor;

    /// Returns list currently selected entity ID's.
    QList<entity_id_t> GetSelectedEntities() const;

    /// Returns list of currently selected components.
    QList<QPair<entity_id_t, ComponentItem *> > GetSelectedComponents() const;

    /// Returns currently selected entities as XML string.
    QString GetSelectionAsXml() const;

private slots:
    /// Opens selected entities in EC editor window. An exisiting editor window is used if possible.
    void Edit();

    /// Opens selected entities in EC editor window. New editor window is created each time.
    void EditInNew();

    /// Renames selected entity.
    void Rename();

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
