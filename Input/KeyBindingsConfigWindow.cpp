#include <QVBoxLayout>
#include <QTreeWidget>
#include <QUiLoader>
#include <QPushButton>
#include <QFile>
#include <QKeySequence>
#include <cassert>

#include "Framework.h"
#include "Input.h"

#include "KeyBindingsConfigWindow.h"

#include <QObject>
#include <QStringList>
#include <QMessageBox>
#include <QList>
#include <QMap>
#include <QEvent>
#include <QKeyEvent>

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
    configList->installEventFilter(this);

    QPushButton *apply = findChild<QPushButton*>("pushButtonApply");
    connect(apply, SIGNAL(pressed()), this, SLOT(ApplyKeyConfig()));

    connect(configList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    connect(configList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ConfigListAdjustEditable(QTreeWidgetItem *, int)));
    PopulateBindingsList();

    setWindowTitle(tr("Actions"));
    setAttribute(Qt::WA_DeleteOnClose);
}

void KeyBindingsConfigWindow::CloseWindow()
{
}

void KeyBindingsConfigWindow::ApplyKeyConfig()
{
    ExtractBindingsList();

    bool found = false;
    QStringList found_sequences;
    for ( std::map<std::string, QKeySequence>::const_iterator it = editedActions.begin();
      it != editedActions.end(); ++it )
    {
        for ( std::map<std::string, QKeySequence>::const_iterator jt = editedActions.begin();
         jt != editedActions.end(); ++jt )
        {
            if(it != jt)
            {
                if(it->second == jt->second)
                {
                    if(!found_sequences.contains(it->second.toString()))
                    {
                        found_sequences.append(it->second.toString());
                    }
                    found = true;
                    break;
                }
            }
        }
    }
     if(found)
     {
        if(!QMessageBox::question(this, tr("Bindings Warning? -- Naali"), 
                    tr("Same shortcut '%1' has been set on multiple actions! Do you wish to continue?").arg(found_sequences.join(",")), 
                    tr("&Yes"), tr("&No"),
                    QString::null, 0, 1 ))
                    {
                        framework->GetInput()->SetKeyBindings(editedActions);
                        framework->GetInput()->SaveKeyBindingsToFile();
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

    const Input::KeyActionsMap &keyActions = framework->GetInput()->GetKeyBindings();

    for(Input::KeyActionsMap::const_iterator iter = keyActions.begin(); iter != keyActions.end(); ++iter)
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
}

void KeyBindingsConfigWindow::ButtonCancel()
{
    framework->GetInput()->LoadKeyBindingsFromFile();
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