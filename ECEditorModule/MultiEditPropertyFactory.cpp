#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditPropertyFactory.h"
#include "qteditorfactory.h"
#include <QLayout>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    MultiEditPropertyFact::MultiEditPropertyFact(QObject *parent):
        QtAbstractEditorFactory<MultiEditPropertyManager>(parent)
    {
        
    }

    MultiEditPropertyFact::~MultiEditPropertyFact()
    {
        qDeleteAll(editorToProperty_.keys());
    }

    void MultiEditPropertyFact::connectPropertyManager(MultiEditPropertyManager *manager)
    {
        /*QObject::connect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
                            this, SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));*/
    }

    QWidget *MultiEditPropertyFact::createEditor(MultiEditPropertyManager *manager, QtProperty *property, QWidget *parent)
    {
        QPushButton *multiEditButton = new QPushButton(parent);
        QInputDialog *dialog = new QInputDialog(parent);
        QStringList attributes = manager->AttributeValue(property);
        dialog->setComboBoxItems(attributes);
        dialog->setInputMode(QInputDialog::TextInput);
        dialog->setComboBoxEditable(true);
        QObject::connect(multiEditButton, SIGNAL(clicked()), dialog, SLOT(open()));
        multiEditButton->setText(QString("(%1 values)").arg(attributes.size()));

        createdEditors_[property] = dialog;
        editorToProperty_[dialog] = property;
        QObject::connect(dialog, SIGNAL(textValueSelected(const QString &)), 
                         this, SLOT(DialogValueSelected(const QString &)));
        QObject::connect(this, SIGNAL(ValueSelected(QtProperty *, const QString &)), 
                         manager, SLOT(SetValue(QtProperty *, const QString &)));
        QObject::connect(dialog, SIGNAL(destroyed(QObject *)),
                         this, SLOT(EditorDestroyed(QObject *)));

        return multiEditButton;
    }

    void MultiEditPropertyFact::disconnectPropertyManager(MultiEditPropertyManager *manager)
    {
        /*QObject::disconnect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
                               this, SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));*/
    }

    void MultiEditPropertyFact::DialogValueSelected(const QString &value)
    {
        QInputDialog *dialog = dynamic_cast<QInputDialog *>(QObject::sender());
        if(!dialog)
            return;
        if(!editorToProperty_.contains(dialog))
            return;
        QtProperty *property = const_cast<QtProperty *>(editorToProperty_[dialog]);

        emit ValueSelected(property, value);
    }

    void MultiEditPropertyFact::EditorDestroyed(QObject *object)
    {
        QMap<QDialog *, const QtProperty *>::ConstIterator iter = editorToProperty_.constBegin();
        while (iter != editorToProperty_.constEnd()) 
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
}