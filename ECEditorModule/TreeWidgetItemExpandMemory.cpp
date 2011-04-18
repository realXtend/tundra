/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TreeWidgetItemExpandMemory.cpp
 *  @brief  Utility class for keeping track of expanded items at a tree widget.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "TreeWidgetItemExpandMemory.h"
#include "Framework.h"
#include "ConfigAPI.h"

#include <QTreeWidget>

TreeWidgetItemExpandMemory::TreeWidgetItemExpandMemory(const char *group, Framework *framework) :
    framework_(framework),
    groupName(group)
{
    Load();
}

TreeWidgetItemExpandMemory::~TreeWidgetItemExpandMemory()
{
    Save();
}

void TreeWidgetItemExpandMemory::ExpandItem(QTreeWidget *treeWidget, const QTreeWidgetItem *item) const
{
    if (!item)
        return;

    if (items.find(GetIndentifierText(item)) != items.end())
        treeWidget->expandItem(item);
    else
        treeWidget->collapseItem(item);

    for(int i = 0; i < item->childCount(); ++i)
        ExpandItem(treeWidget, item->child(i));
}

QString TreeWidgetItemExpandMemory::GetIndentifierText(const QTreeWidgetItem *item) const
{
    QList<QTreeWidgetItem *> items;
    const QTreeWidgetItem *c = item;
    QTreeWidgetItem *p = 0;
    while((p = c->parent()) != 0)
    {
        items.push_front(p);
        c = p;
    }

    QString indentifier;
    foreach(QTreeWidgetItem *i, items)
        indentifier.append(i->text(0) + ".");
    indentifier.append(item->text(0));

    return indentifier;
}

void TreeWidgetItemExpandMemory::Load()
{
    QStringList setting = framework_->Config()->Get("uimemory", "tree state", "expanded").toString().split("|");
    foreach(QString s, setting)
        if (!s.isEmpty())
            items.insert(s);
}

void TreeWidgetItemExpandMemory::Save()
{
    QString state = QString::fromStdString(ToString());
    if (!state.isEmpty())
        framework_->Config()->Set("uimemory", "tree state", "expanded", state);
}

void TreeWidgetItemExpandMemory::HandleItemExpanded(QTreeWidgetItem *item)
{
    QString idText = GetIndentifierText(item);
    int idx = idText.lastIndexOf('.');
    if ((!idText.isEmpty()) && (idx != idText.size() - 1))
        items.insert(idText);
}

void TreeWidgetItemExpandMemory::HandleItemCollapsed(QTreeWidgetItem *item)
{
    QString idText = GetIndentifierText(item);
    int idx = idText.lastIndexOf('.');
    if ((!idText.isEmpty()) && (idx != idText.size() - 1))
        items.remove(idText);
}

std::string TreeWidgetItemExpandMemory::ToString() const
{
    std::string ret;
    int idx = 0;
    foreach(QString item, items)
    {
        ret.append(item.toStdString());
        if (idx < items.size() - 1)
            ret.append("|");
        ++idx;
    }

    return ret;
}

