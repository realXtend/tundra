/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.h
 *  @brief  
 *
 *          Detailed.
 */

#ifndef incl_SceneStructureModule_AssetsWindow_h
#define incl_SceneStructureModule_AssetsWindow_h

#include "ForwardDefines.h"

#include <QWidget>

class QTreeWidget;

///
/**
*/
class AssetsWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
    */
    explicit AssetsWindow(Foundation::Framework *fw);

    /// Destructor.
    ~AssetsWindow();

private:
    ///
    void PopulateTreeWidget();

    Foundation::Framework *framework; ///< Framework pointer.
    QTreeWidget *treeWidget; ///< Tree widget showing the assets.

private slots:
    /// Searches for items containing @c text (case-insensitive) and toggles their visibility.
    /** If match is found the item is set visible and expanded, otherwise it's hidden.
        @param filter Text used as a filter.
    */
    void Search(const QString &filter);

    /// Expands or collapses the whole tree view, depending of the previous action.
    void ExpandOrCollapseAll();
};

#endif
