/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetTreeWidget.h
 *  @brief  Tree widget showing all available assets.
 */

#ifndef incl_SceneStructureModule_AssetTreeWidget_h
#define incl_SceneStructureModule_AssetTreeWidget_h

#include <QTreeWidget>

#include "ForwardDefines.h"

class QMenu;

/// Tree widget showing all available assets.
class AssetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework pointer.
        @param parent Parent widget.
    */
    AssetTreeWidget(Foundation::Framework *fw, QWidget *parent = 0);

    /// Destructor.
    virtual ~AssetTreeWidget();

protected:
    /// QWidget override. Show context menu with assets-spesific actions.
    void contextMenuEvent(QContextMenuEvent *e);

    /// QWidget override.
    //void dragEnterEvent(QDragEnterEvent *e);

    /// QWidget override.
    //void dragMoveEvent(QDragMoveEvent *e);

    /// QWidget override.
    //void dropEvent(QDropEvent *e);

private:
    /// Creates and adds applicable actions to the right-click context menu.
    /** @param menu Context menu.
    */
    void AddAvailableActions(QMenu *menu);

    Foundation::Framework *framework; ///> Framework.
    QMenu *contextMenu; ///< Right-click context menu.
};

#endif
