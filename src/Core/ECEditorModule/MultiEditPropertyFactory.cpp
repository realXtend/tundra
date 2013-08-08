// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditPropertyFactory.h"
#include "EditorButtonFactory.h"

#include <QLayout>
#include <qteditorfactory.h>

#include "MemoryLeakCheck.h"

MultiEditButton::MultiEditButton(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout);

    button = new QPushButton(this);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(button);
    setFocusProxy(button);
}

MultiEditPropertyFactory::MultiEditPropertyFactory(QObject *parent):
    QtAbstractEditorFactory<MultiEditPropertyManager>(parent)
{
}

MultiEditPropertyFactory::~MultiEditPropertyFactory()
{
    qDeleteAll(editorToProperty_.keys());
}

/// @todo Remove?
void MultiEditPropertyFactory::connectPropertyManager(MultiEditPropertyManager * /*manager*/)
{
/*
    connect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
        SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));
*/
}

QWidget *MultiEditPropertyFactory::createEditor(MultiEditPropertyManager *manager, QtProperty *property, QWidget *parent)
{
    //QPushButton *multiEditButton = new QPushButton(parent);
    MultiEditButton *multiEditButton = new MultiEditButton(parent);
    buttonFactory = new EditorButtonFactory(parent);
    multiEditButton->layout()->addWidget(buttonFactory);

    /// \bug QInputContext has a bug where if you have a QInputDialog in a QGraphicsView and close the dialog, the QInputContext
    /// will access a dangling pointer and crash. This bug can be avoided by never freeing the QInputDialog (and leaking),
    /// which allows QInputContext::setFocusWidget to go through. The following commented out line would be preferred, but
    /// at the moment is not possible. See http://clb.demon.fi/dump/QWinInputContextUpdateCrash.png . -jj.
//        QInputDialog *dialog = new QInputDialog(parent);
    // Now as a workaround, create the input dialog without a parent so that we won't crash.
    QInputDialog *dialog = new QInputDialog();

    QStringList attributes = manager->AttributeValue(property);
    dialog->setComboBoxItems(attributes);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setComboBoxEditable(true);
    connect(multiEditButton->button, SIGNAL(clicked()), dialog, SLOT(open()));
    multiEditButton->button->setText(QString("(%1 values)").arg(attributes.size()));

    createdEditors_[property] = dialog;
    editorToProperty_[dialog] = property;

    connect(dialog, SIGNAL(textValueSelected(const QString &)), SLOT(DialogValueSelected(const QString &)));
    connect(this, SIGNAL(ValueSelected(QtProperty *, const QString &)), manager, SLOT(SetValue(QtProperty *, const QString &)));
    connect(dialog, SIGNAL(destroyed(QObject *)), SLOT(EditorDestroyed(QObject *)));

    connect(dialog, SIGNAL(textValueChanged(QString)), SIGNAL(TextEdited(QString)));

    emit EditorCreated(property, this);

    return multiEditButton;
}

/// @todo Remove?
void MultiEditPropertyFactory::disconnectPropertyManager(MultiEditPropertyManager * /*manager*/)
{
/*
    disconnect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
        SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));
*/
}

void MultiEditPropertyFactory::DialogValueSelected(const QString &value)
{
    QInputDialog *dialog = dynamic_cast<QInputDialog *>(sender());
    if(!dialog)
        return;
    if(!editorToProperty_.contains(dialog))
        return;
    QtProperty *property = const_cast<QtProperty *>(editorToProperty_[dialog]);

    emit ValueSelected(property, value);
}

void MultiEditPropertyFactory::EditorDestroyed(QObject *object)
{
    QMap<QDialog *, const QtProperty *>::ConstIterator iter = editorToProperty_.constBegin();
    while(iter != editorToProperty_.constEnd())
    {
        if (iter.key() == object)
        {
            QDialog *dialog = iter.key();
            const QtProperty *property = iter.value();
            editorToProperty_.remove(dialog);
            createdEditors_.remove(property);
            break;
        }
        iter++;
    }
}
