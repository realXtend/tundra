/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   TreeWidgetUtils.h
 *  @brief  Tree widget utility functions.
 */

#pragma once

#include "ECEditorModuleApi.h"

#include <QSet>
#include <QString>

/// Searches for items containing @c filter (case-insensitive) in the item's @c column and toggles their visibility.
/** If match is found the item is set visible and expanded (if filter >= 3 chars), otherwise it's hidden.
    If @c filter begins with '!', negation search is performed, i.e. every item containing the filter is hidden instead.
    @param Target tree widget for the search
    @param Which column's text is used.
    @param Text used as a filter. If an empty string, all items in the tree widget are set visible. 
    @param If true QString::equals(filter, @c sensitivity) is used. If false QString::contains(filter, @c sensitivity) is used. 
    @param Case sensitivity for string matching. 
    @param If found items should be automatically expanded from the root to the matched child. 
    @return List of matched child items. */
QSet<QTreeWidgetItem*> ECEDITOR_MODULE_API TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QString &filter, bool exactMatch = false, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive, bool expandItems = true);
QSet<QTreeWidgetItem*> ECEDITOR_MODULE_API TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QStringList &filters, bool exactMatch = false, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive, bool expandItems = true); /**< @overload */

/// Expands or collapses the whole tree view, depending on the previous action.
/** @param treeWidget Target tree widget for the action.
    @return bool True all items are expanded, false if all items are collapsed. */
bool ECEDITOR_MODULE_API TreeWidgetExpandOrCollapseAll(QTreeWidget *treeWidget);

/// Sets the check state of all items in the @c treeWidget.
/** Sorting and updates are disabled before setting the state and enabled afterwards.
    @param treeWidget Target tree widget for the action.
    @param column Column for which the @c state is set.
    @param state Check state. */
void ECEDITOR_MODULE_API TreeWidgetSetCheckStateForAllItems(QTreeWidget *treeWidget, int column, Qt::CheckState state);
