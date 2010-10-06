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
//#include <QAction>
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

/// This is helper class to make translations work with QActions.
/*
class SceneEditAction : public QAction
{
    Q_OBJECT

public:
    SceneEditAction(QWidget *parent = 0) : QAction(parent) {}
    SceneEditAction(const QString& text, QWidget *parent = 0) : QAction(text, parent), originalText(text) {}
    QString GetText() const { return originalText; }

private:
    QString originalText;
};
*/

/// Tree widget item with which has ID number
class SceneTreeWidgetItem : public QTreeWidgetItem
{
public:
    ///
    /** @param
    */
    explicit SceneTreeWidgetItem(entity_id_t entityId) : id(entityId) {}

    /// Entity ID.
    entity_id_t id;
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

private slots:
    ///
    void Edit();

    ///
    void EditInNew();

    ///
    void Rename();

    ///
    void Delete();

    ///
    void Copy();

    ///
    void Paste();
};

#endif
