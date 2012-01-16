// For conditions of distribution and use, see copyright notice in LICENSE

#include "DebugOperatorNew.h"

#include "KeyBindingsConfigWindow.h"
#include "InputAPI.h"

#include "Framework.h"
#include "Application.h"

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QUiLoader>
#include <QPushButton>
#include <QFile>
#include <QKeySequence>
#include <QObject>
#include <QStringList>
#include <QMessageBox>
#include <QList>
#include <QMap>
#include <QEvent>
#include <QKeyEvent>

#include <cassert>

#include "MemoryLeakCheck.h"

KeyBindingsConfigWindow::KeyBindingsConfigWindow(Framework *fw) :
    framework(fw),
    configList(0)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/KeyBindingsConfig.ui");
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
    configList->installEventFilter(this);

    QPushButton *apply = findChild<QPushButton*>("pushButtonApply");
    connect(apply, SIGNAL(pressed()), this, SLOT(ApplyKeyConfig()));

    connect(configList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    connect(configList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    PopulateBindingsList();

    setWindowTitle(tr("Key bindings"));
    setAttribute(Qt::WA_DeleteOnClose);
}

KeyBindingsConfigWindow::~KeyBindingsConfigWindow()
{
    Clear();
}

void KeyBindingsConfigWindow::ApplyKeyConfig()
{
    ExtractBindingsList();

    QStringList conflictingSequences;
    for(InputAPI::KeyBindingMap::const_iterator it = editedActions.begin(); it != editedActions.end(); ++it)
    {
        InputAPI::KeyBindingMap::const_iterator jt = it;
        ++jt;
        for(;jt != editedActions.end(); ++jt)
            if(it != jt && it.value() == jt.value() && it.value() == jt.value() && !conflictingSequences.contains(it.value()))
            {
                conflictingSequences.append(it.value());
                break;
            }
    }

    if (conflictingSequences.size() > 0)
    {
        if(!QMessageBox::question(this, tr("Bindings Warning?"), 
            tr("Same shortcut '%1' has been set on multiple actions! Do you wish to continue?").arg(conflictingSequences.join(",")), 
            tr("&Yes"), tr("&No"),
            QString::null, 0, 1))
        {
            framework->Input()->SetKeyBindings(editedActions);
            framework->Input()->SaveKeyBindingsToFile();
        }
    }
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
        editedActions[item->text(0)] = QKeySequence::fromString(item->text(1), QKeySequence::NativeText);
    }
}

void KeyBindingsConfigWindow::Clear()
{
    for(uint i = 0; i < (uint)configList->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = configList->topLevelItem(i);
        delete item;
    }
    configList->clear();
}

void KeyBindingsConfigWindow::PopulateBindingsList()
{
    assert(configList);
    if (!configList)
        return;

    Clear();

    const InputAPI::KeyBindingMap &keyActions = framework->Input()->KeyBindings();
    for(InputAPI::KeyBindingMap::const_iterator iter = keyActions.begin(); iter != keyActions.end(); ++iter)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(iter.key()));
        item->setText(1, iter.value().toString(QKeySequence::NativeText));
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        configList->addTopLevelItem(item);
    }
    configList->resizeColumnToContents(0);
}

void KeyBindingsConfigWindow::ButtonOK()
{
    ApplyKeyConfig();
}

void KeyBindingsConfigWindow::ButtonCancel()
{
    framework->Input()->LoadKeyBindingsFromFile();
}

bool KeyBindingsConfigWindow::eventFilter(QObject *obj, QEvent *event)
{
    QTreeWidgetItem *current_item = configList->currentItem();
    if (event->type() == QEvent::KeyPress) 
    {
        QKeyEvent *key_event = dynamic_cast<QKeyEvent*>(event);
        if (key_event && current_item)
        {
            if (key_event->isAutoRepeat())
                return true;

            if (key_event->key() == Qt::Key_Backspace ||
                key_event->key() == Qt::Key_Return ||
                key_event->key() == Qt::Key_Enter )
                return QWidget::eventFilter(obj, event);

            if (key_event->modifiers() != Qt::NoModifier && !key_event->isAutoRepeat())
            {
                QKeySequence key_text(key_event->modifiers());
                QString text = key_text.toString();
                QString insert_text = text.remove(text.count() - 1, 1);
                current_item->setText(1, insert_text);
                key_event->accept();
                return true;
            }
            else
            {
                if(key_event->key() == Qt::Key_Up ||
                    key_event->key() == Qt::Key_Down || 
                    key_event->key() == Qt::Key_Tab ||
                    key_event->key() == Qt::Key_PageUp ||
                    key_event->key() == Qt::Key_PageDown)
                {
                    key_event->accept();
                    return true;
                }
            }
        }
    }
    else
    {
        if (event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *key_event = dynamic_cast<QKeyEvent*>(event);
            if (key_event && current_item)
            {
                if (key_event->isAutoRepeat())
                return true;

                if (key_event->key() == Qt::Key_Backspace ||
                    key_event->key() == Qt::Key_Return ||
                    key_event->key() == Qt::Key_Enter || 
                    SpecialKeyPressChecker(key_event->key()))
                    return QWidget::eventFilter(obj, event);

                if (key_event->modifiers() == Qt::NoModifier || key_event->key() != Qt::Key_unknown)
                {
                    QString item_text = current_item->text(1);
                    if(item_text.length() < 2)
                    {
                        current_item->setText(1, "");
                        current_item->setText(1, QKeySequence(key_event->key()).toString());
                        key_event->accept();
                        return true;
                    }
                    else 
                    {
                        QStringList items = item_text.split("+");
                        if(!items.isEmpty())
                        {
                            if(items.last().length() < 2)
                            {
                                current_item->setText(1, QKeySequence(key_event->key()).toString());
                                key_event->accept();
                                return true;
                            }
                            else
                            {
                                int already_there = 0;
                                for(int i = 0; i < items.length(); i++)
                                {
                                    if(items.value(i) == QKeySequence(key_event->key()).toString())
                                    {
                                        already_there = 1;
                                        break;
                                    }
                                }
                                if(already_there)
                                {
                                    current_item->setText(1, QKeySequence(key_event->key()).toString());
                                    key_event->accept();
                                    return true;
                                }
                                else
                                {
                                    current_item->setText(1, item_text + "+" + QKeySequence(key_event->key()).toString());
                                    key_event->accept();
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

bool KeyBindingsConfigWindow::SpecialKeyPressChecker(int pressed_key)
{
    bool result = false;
    switch(pressed_key)
    {
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Meta:
        case Qt::Key_AltGr:
            result = true;
            break;
    }
    return result;
}