#include <QVBoxLayout>
#include <QTreeWidget>
#include <QUiLoader>
#include <QPushButton>
#include <QFile>
#include <cassert>

#include "Framework.h"
#include "QtInputService.h"

#include "KeyBindingsConfigWindow.h"

void KeyBindingsConfigWindow::ShowWindow()
{
    QUiLoader loader;
    QFile file("./data/ui/KeyBindingsConfig.ui");
    file.open(QFile::ReadOnly);
    QWidget *contents_widget_ = loader.load(&file, this);
    assert(contents_widget_);
    file.close();

    if (!contents_widget_)
        return;

    QVBoxLayout *layout = new QVBoxLayout;
    assert(layout);
    layout->addWidget(contents_widget_);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    configList = findChild<QTreeWidget*>("configList");

    QPushButton *apply = findChild<QPushButton*>("pushButtonApply");
    connect(apply, SIGNAL(pressed()), this, SLOT(ApplyKeyConfig()));
    QPushButton *ok = findChild<QPushButton*>("pushButtonOK");
    connect(ok, SIGNAL(pressed()), this, SLOT(ButtonOK()));
    QPushButton *cancel = findChild<QPushButton*>("pushButtonCancel");
    connect(cancel, SIGNAL(pressed()), this, SLOT(ButtonCancel()));

    connect(configList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    connect(configList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    PopulateBindingsList();

    setAttribute(Qt::WA_DeleteOnClose);
}

void KeyBindingsConfigWindow::CloseWindow()
{
}

void KeyBindingsConfigWindow::ApplyKeyConfig()
{
    ExtractBindingsList();
    dynamic_cast<QtInputService&>(framework->Input()).SetKeyBindings(editedActions);
    dynamic_cast<QtInputService&>(framework->Input()).SaveKeyBindingsToFile();
}

/// Read more from http://www.qtcentre.org/threads/26689-QTableWidget-one-column-editable
void KeyBindingsConfigWindow::ConfigListAdjustEditable(QTreeWidgetItem *item, int column)
{
    if (column == 1)
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
    else
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void KeyBindingsConfigWindow::ExtractBindingsList()
{
    for(int i = 0; i < configList->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = configList->topLevelItem(i);
        std::string actionName = item->text(0).toStdString();
        QKeySequence shortcut = QKeySequence::fromString(item->text(1), QKeySequence::NativeText);
        editedActions[actionName] = shortcut;
    }
}

void KeyBindingsConfigWindow::PopulateBindingsList()
{
    assert(configList);
    if (!configList)
        return;

    configList->clear();

    const QtInputService::KeyActionsMap &keyActions = dynamic_cast<QtInputService&>(framework->Input()).GetKeyBindings();

    for(QtInputService::KeyActionsMap::const_iterator iter = keyActions.begin(); iter != keyActions.end(); ++iter)
    {
        ///\todo Fix - this leaks?
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(iter->first.c_str())));
        item->setText(1, iter->second.toString(QKeySequence::NativeText));
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        configList->addTopLevelItem(item);
    }
    configList->resizeColumnToContents(0);
}

void KeyBindingsConfigWindow::ButtonOK()
{
    ApplyKeyConfig();

    close();
}

void KeyBindingsConfigWindow::ButtonCancel()
{
    dynamic_cast<QtInputService&>(framework->Input()).LoadKeyBindingsFromFile();    

    close();
}

