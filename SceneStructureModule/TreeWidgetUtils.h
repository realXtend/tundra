/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TreeWidgetUtils.h
 *  @brief  Tree widget utility functions.
 */

#ifndef incl_SceneStructureModule_TreeWidgetUtils_h
#define incl_SceneStructureModule_TreeWidgetUtils_h

/// Searches for items containing @c filter (case-insensitive) in the item's @c column and toggles their visibility.
/** If match is found the item is set visible and expanded (if filter >= 3 chars), otherwise it's hidden.
    @param treeWidget Target tree widget for the action.
    @param column Which column's text is used.
    @param filter Text used as a filter.
*/
void TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QString &filter);

/// Expands or collapses the whole tree view, depending on the previous action.
/** @param treeWidget Target tree widget for the action.
*/
void TreeWidgetExpandOrCollapseAll(QTreeWidget *treeWidget);

/// Sets the check state of all items in the @c treeWidget.
/** Sorting and updates are disabled before setting the state and enabled afterwards.
    @param treeWidget Target tree widget for the action.
    @param column Column for which the @c state is set.
    @param state Check state.
*/
void TreeWidgetSetCheckStateForAllItems(QTreeWidget *treeWidget, int column, Qt::CheckState state);

#endif
